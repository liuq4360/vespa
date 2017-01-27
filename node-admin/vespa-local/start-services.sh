#!/bin/bash
# Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

# TODO: This file duplicates a file in the base image (with local modifications).
# Update the file there.

# BEGIN environment bootstrap section
# Do not edit between here and END as this section should stay identical in all scripts

findpath () {
    myname=${0}
    mypath=${myname%/*}
    myname=${myname##*/}
    if [ "$mypath" ] && [ -d "$mypath" ]; then
        return
    fi
    mypath=$(pwd)
    if [ -f "${mypath}/${myname}" ]; then
        return
    fi
    echo "FATAL: Could not figure out the path where $myname lives from $0"
    exit 1
}

COMMON_ENV=libexec/vespa/common-env.sh

source_common_env () {
    if [ "$VESPA_HOME" ] && [ -d "$VESPA_HOME" ]; then
        # ensure it ends with "/" :
        VESPA_HOME=${VESPA_HOME%/}/
        export VESPA_HOME
        common_env=$VESPA_HOME/$COMMON_ENV
        if [ -f "$common_env" ]; then
            . $common_env
            return
        fi
    fi
    return 1
}

findroot () {
    source_common_env && return
    if [ "$VESPA_HOME" ]; then
        echo "FATAL: bad VESPA_HOME value '$VESPA_HOME'"
        exit 1
    fi
    if [ "$ROOT" ] && [ -d "$ROOT" ]; then
        VESPA_HOME="$ROOT"
        source_common_env && return
    fi
    findpath
    while [ "$mypath" ]; do
        VESPA_HOME=${mypath}
        source_common_env && return
        mypath=${mypath%/*}
    done
    echo "FATAL: missing VESPA_HOME environment variable"
    echo "Could not locate $COMMON_ENV anywhere"
    exit 1
}

findroot

# END environment bootstrap section

export LC_ALL=C

function wait_for_network_up {
    while true
    do
        for config_server_host in $(echo $CONFIG_SERVER_ADDRESS | tr "," " ")
        do
            ping -c 1 -W 3 $config_server_host && return
            sleep 1
        done
    done
}

if [ -z  $CONFIG_SERVER_ADDRESS ]
then
    echo "CONFIG_SERVER_ADDRESS must be set."
    exit -1
fi

chown yahoo $VESPA_HOME/var/jdisc_container

# Local modification; fixes ownership issues for vespa node running in container.
chown yahoo $VESPA_HOME/var/zookeeper

if [ -d "$VESPA_HOME/logs" ]
then
    chmod 1777 $VESPA_HOME/logs
fi

yinst set services.addr_configserver=$CONFIG_SERVER_ADDRESS
wait_for_network_up
yinst start services
logfmt -n -f