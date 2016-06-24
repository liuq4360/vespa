#!/usr/bin/env python
# Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

# Quick and dirty script to set up routable ip address for docker container
# Remove when docker releases a plugin api to configure networking.
# TODO: Refactor for readability


from __future__ import print_function


import hashlib
import ipaddress
import os
import sys


from pyroute2 import IPRoute
from pyroute2 import NetNS
from pyroute2.netlink import NetlinkError
from socket import AF_INET
from socket import gethostname


def create_directory_ignore_exists(path, permissions):
    if not os.path.isdir(path):
        os.mkdir(path, permissions)

        
def create_symlink_ignore_exists(path_to_point_to, symlink_location):
    if not os.path.islink(symlink_location):
        os.symlink(path_to_point_to, symlink_location)


def get_attribute(struct_with_attrs, name):
    try:
        matching_values = [attribute[1] for attribute in struct_with_attrs['attrs'] if attribute[0] == name]
        if len(matching_values) == 0:
            raise RuntimeError("No such attribute: %s" % name)
        elif len(matching_values) != 1:
            raise RuntimeError("Multiple values for attribute %s" %name)
        else:
            return matching_values[0]
    except Exception as e:
        raise RuntimeError("Couldn't find attribute %s for value: %s" % (name, struct_with_attrs), e)

def network(ipv4_address):
    ip = ipaddress.ip_network(unicode(get_attribute(ipv4_address, 'IFA_ADDRESS')))
    prefix = int(ipv4_address['prefixlen'])
    return ip.supernet(new_prefix = prefix)

def net_namespace_path(pid):
    return "/host/proc/%d/ns/net" % pid

def generate_mac_address(base_host_name, ip_address):
    hash = hashlib.sha1()
    hash.update(base_host_name)
    hash.update(ip_address)
    digest = hash.digest()
    # For a mac address, we only need six bytes.
    six_byte_digest = digest[:6]
    mac_address_bytes = bytearray(six_byte_digest)

    # Set 'unicast'
    mac_address_bytes[0] &= 0b11111110

    # Set 'local'
    mac_address_bytes[0] |= 0b00000010

    mac_address = ':'.join('%02x' % n for n in mac_address_bytes)
    return mac_address

def get_net_namespace_for_pid(pid):
    net_ns_path = net_namespace_path(pid)
    if not os.path.isfile(net_ns_path):
        raise RuntimeError("No such net namespace %s" % net_ns_path )
    create_directory_ignore_exists("/var/run/netns", 0766)
    create_symlink_ignore_exists(net_ns_path,  "/var/run/netns/%d" % pid)
    return NetNS(str(pid))

# ipv4 address format: {
#     'index': 3,
#     'family': 2,
#     'header': {
#         'pid': 15,
#         'length': 88,
#         'flags': 2,
#         'error': None,
#         'type': 20,
#         'sequence_number': 256
#     },
#     'flags': 128,
#     'attrs': [
#         ['IFA_ADDRESS', '10.0.2.15'],
#         ['IFA_LOCAL', '10.0.2.15'],
#         ['IFA_BROADCAST', '10.0.2.255'],
#         ['IFA_LABEL', 'eth0'],
#         ['IFA_FLAGS', 128],
#         [
#             'IFA_CACHEINFO',
#             {
#                 'ifa_valid': 4294967295,
#                 'tstamp': 2448,
#                 'cstamp': 2448,
#                 'ifa_prefered': 4294967295
#             }
#         ]
#     ],
#     'prefixlen': 24,
#     'scope': 0,
#     'event': 'RTM_NEWADDR'
# }
def ip_with_most_specific_network_for_address(address, ipv4_ips):
    host_ips_with_network_matching_address = [host_ip for host_ip in ipv4_ips if address in network(host_ip)]

    host_ip_best_match_for_address = None
    for host_ip in host_ips_with_network_matching_address:
        if not host_ip_best_match_for_address:
            host_ip_best_match_for_address = host_ip
        elif host_ip['prefixlen'] < host_ip_best_match_for_address['prefixlen']:
            host_ip_best_match_for_address = host_ip

    if not host_ip_best_match_for_address:
        raise RuntimeError("No matching ip address for %s, candidates are on networks %s" % (address, ', '.join([str(network(host_ip)) for host_ip in ipv4_ips])))
    return host_ip_best_match_for_address

ipr = IPRoute()

def delete_interface_by_name(interface_name):
    for interface_index in ipr.link_lookup(ifname=interface_name):
        ipr.link('delete', index=interface_index)

def create_interface_in_namespace(network_namespace, ip_address_textual, interface_name, link_device_index):
    mac_address = generate_mac_address(
        base_host_name=gethostname(),
        ip_address=ip_address_textual)

    # For traceability.
    with open('/tmp/container_mac_address_' + ip_address_textual, 'w') as f:
        f.write(mac_address)

    # result = [{
    #     'header': {
    #         'pid': 240,
    #         'length': 36,
    #         'flags': 0,
    #         'error': None,
    #         'type': 2,
    #         'sequence_number': 256
    #     },
    #     'event': 'NLMSG_ERROR'
    # }]
    result = network_namespace.link_create(
        ifname=interface_name,
        kind='macvlan',
        link=link_device_index,
        macvlan_mode='bridge',
        address=mac_address)
    if result[0]['header']['error']:
        raise RuntimeError("Failed creating link, result = %s" % result )

    index_of_created_interface = network_namespace.link_lookup(ifname=interface_name)[0]
    return index_of_created_interface

def index_of_interface_in_namespace(interface_name, namespace):
    interface_index_list = namespace.link_lookup(ifname=interface_name)
    if not interface_index_list:
        return None
    assert len(interface_index_list) == 1
    return interface_index_list[0]

def move_interface(src_interface_index, dest_namespace, dest_namespace_pid, dest_interface_name):
    ipr.link('set',
             index=src_interface_index,
             net_ns_fd=str(dest_namespace_pid),
             ifname=dest_interface_name)

    new_interface_index = index_of_interface_in_namespace(interface_name=dest_interface_name,
                                                          namespace=dest_namespace)
    if not new_interface_index:
        raise RuntimeError("Concurrent modification to network interfaces")
    return new_interface_index

def set_ip_address(net_namespace, interface_index, ip_address, network_prefix_length):
    ip_already_configured = False

    for existing_ip in net_namespace.get_addr(index=interface_index, family = AF_INET):
        existing_ip_address = get_attribute(existing_ip, 'IFA_ADDRESS')
        existing_ip_prefixlen = existing_ip['prefixlen']
        is_same_address = ipaddress.ip_address(unicode(existing_ip_address)) == ip_address
        is_same_netmask = existing_ip_prefixlen == network_prefix_length
        if is_same_address and is_same_netmask:
            ip_already_configured = True
        else:
            print("Deleting old ip address. %s/%s" % (existing_ip_address, existing_ip_prefixlen))
            result_of_remove = net_namespace.addr('remove',
                                                  index=interface_index,
                                                  address=existing_ip_address,
                                                  mask=existing_ip_prefixlen)
            print(result_of_remove)

    if not ip_already_configured:
        try:
            net_namespace.addr('add',
                               index=interface_index,
                               address=str(ip_address),
                               # broadcast='192.168.59.255',
                               mask=network_prefix_length)
        except NetlinkError as e:
            if e.code == 17:  # File exists, i.e. address is already added
                pass

def get_default_route(net_namespace):
    # route format: {
    #     'family': 2,
    #     'dst_len': 0,
    #     'proto': 3,
    #     'tos': 0,
    #     'event': 'RTM_NEWROUTE',
    #     'header': {
    #         'pid': 43,
    #         'length': 52,
    #         'flags': 2,
    #         'error': None,
    #         'type': 24,
    #         'sequence_number': 255
    #     },
    #     'flags': 0,
    #     'attrs': [
    #         ['RTA_TABLE', 254],
    #         ['RTA_GATEWAY', '172.17.42.1'],
    #         ['RTA_OIF', 18]
    #     ],
    #     'table': 254,
    #     'src_len': 0,
    #     'type': 1,
    #     'scope': 0
    # }
    default_routes = net_namespace.get_default_routes(family = AF_INET)
    if len(default_routes) != 1:
        raise RuntimeError("Couldn't find single default route: " + str(default_routes))
    return default_routes[0]


flag_local_mode = "--local"
local_mode = flag_local_mode in sys.argv
if local_mode:
    sys.argv.remove(flag_local_mode)

flag_vm_mode = "--vm"
vm_mode = flag_vm_mode in sys.argv
if vm_mode:
    sys.argv.remove(flag_vm_mode)

if local_mode and vm_mode:
    raise RuntimeError("Cannot specify both --local and --vm")

if len(sys.argv) != 3:
    raise RuntimeError("Usage: %s <container-pid> <ip>" % sys.argv[0])

container_pid_arg = sys.argv[1]
container_ip_arg = sys.argv[2]

try:
    container_pid = int(container_pid_arg)
except ValueError:
    raise RuntimeError("Container pid must be an integer, got %s" % container_pid_arg)
container_ip = ipaddress.ip_address(unicode(container_ip_arg))

host_ns = get_net_namespace_for_pid(1)
container_ns = get_net_namespace_for_pid(container_pid)

all_host_ipv4_ips = host_ns.get_addr(family=AF_INET)
host_ip_best_match_for_container = ip_with_most_specific_network_for_address(address=container_ip,
                                                                             ipv4_ips=all_host_ipv4_ips)
host_device_index_for_container = host_ip_best_match_for_container['index']
container_network_prefix_length = host_ip_best_match_for_container['prefixlen']


# Create new interface for the container.

# The interface to the vespa network are all (in the end) named "vespa". However,
# the container interfaces are prepared in the host network namespace, and so they
# need temporary names to avoid name-clash.
temporary_interface_name_while_in_host_ns = "vespa-tmp-" + container_pid_arg
assert len(temporary_interface_name_while_in_host_ns) <= 15 # linux requirement

container_interface_name = "vespa"
assert len(container_interface_name) <= 15 # linux requirement

# Clean up any leftovers from the past.
delete_interface_by_name(temporary_interface_name_while_in_host_ns)

container_interface_index = index_of_interface_in_namespace(interface_name=container_interface_name,
                                                            namespace=container_ns)
if not container_interface_index:
    # Must be created in the host_ns to have the same lifetime as the host.
    # Otherwise, it will be deleted when the node-admin container stops.
    # (Only temporarily there, moved to the container namespace later.)
    #
    # TODO: Here we're linking against the device with the best matching network.
    # For the sake of argument, as of 2015-12-17, this device is always named
    # 'vespa'. 'vespa' is itself a macvlan bridge linked to the default route's
    # interface (typically eth0 or em1). So could we link against eth0 or em1
    # (or whatever) instead here? What's the difference?
    temporary_interface_index = create_interface_in_namespace(network_namespace=host_ns,
                                                              ip_address_textual=container_ip_arg,
                                                              interface_name=temporary_interface_name_while_in_host_ns,
                                                              link_device_index=host_device_index_for_container)

    # Move interface from host namespace to container namespace, and change name from temporary name.
    # exploit that node_admin docker container shares net namespace with host:
    container_interface_index = move_interface(src_interface_index=temporary_interface_index,
                                               dest_namespace=container_ns,
                                               dest_namespace_pid=container_pid,
                                               dest_interface_name=container_interface_name)


# Set ip address on interface in container namespace.
set_ip_address(net_namespace=container_ns,
               interface_index=container_interface_index,
               ip_address=container_ip,
               network_prefix_length=container_network_prefix_length)


# Activate container interface.

container_ns.link('set', index=container_interface_index, state='up', name=container_interface_name)


if local_mode:
    pass
elif vm_mode:
    # Set the default route to the IP of the host vespa interface (e.g. osx)
    container_ns.route("add", gateway=get_attribute(host_ip_best_match_for_container, 'IFA_ADDRESS'))
else:
    # Set up default route/gateway in container.

    host_default_route = get_default_route(net_namespace=host_ns)

    host_default_route_device_index = get_attribute(host_default_route, 'RTA_OIF')
    host_default_route_gateway = get_attribute(host_default_route, 'RTA_GATEWAY')
    if host_device_index_for_container != host_default_route_device_index:
        raise RuntimeError("Container's ip address is not on the same network as the host's default route."
                           " Could not set up default route for the container.")
    container_gateway = host_default_route_gateway
    container_ns.route("replace", gateway=container_gateway, index=container_interface_index)
