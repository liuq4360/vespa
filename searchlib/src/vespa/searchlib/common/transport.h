// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
// Copyright (C) 1998-2003 Fast Search & Transfer ASA
// Copyright (C) 2003 Overture Services Norway AS


#pragma once


#include <vespa/fastos/fastos.h>

namespace search
{

namespace fs4transport
{

/**
 * Instead of using a 32-bit number to send the 'usehardware' flag, we
 * now use this 32-bit number to send 32 flags. The currently defined flags
 * are as follows:
 * <ul>
 *  <li><b>QFLAG_REPORT_QUEUELEN</b>: Send an extra queue length packet before
 *                                query result packets.</li>
 *  <li><b>QFLAG_ESTIMATE</b>: Indicates that the  query is performed to get
 *                             an estimate of the total number of hits</li>
 *  <li><b>QFLAG_DUMP_FEATURES</b>: Dump detailed ranking information. Note that
 *                             this flag will only be considered when sent in a
 *                             GETDOCSUMSX packet. Is is put here to avoid having
 *                             2 separate query related flag spaces</li>
 *  <li><b>QFLAG_DROP_SORTDATA</b>: Don't return any sort data even if sortspec
 *                             is used.</li>
 *  <li><b>QFLAG_NO_RESULTCACHE</b>: Do not use any result cache. Perform query no matter what.</li>
 * </ul>
 **/
enum queryflags {
    QFLAG_REPORT_QUEUELEN      = 0x00000008,
    QFLAG_ESTIMATE             = 0x00000080,
    QFLAG_DROP_SORTDATA        = 0x00004000,
    QFLAG_REPORT_COVERAGE      = 0x00008000,
    QFLAG_NO_RESULTCACHE       = 0x00010000,
    QFLAG_DUMP_FEATURES        = 0x00040000
};


/**
 * The new PCODE_QUERYRESULTX packet contains a 32-bit field called
 * 'featureflags'. Each bit in that field denotes a separate feature
 * that may be present in the query result packet or not. The comment
 * describing the packet format indicates what data fields depend on
 * what features. Note that after removing the query id and the
 * feature flags from a PCODE_QUERYRESULTX packet it is binary
 * compatible with the PCODE_QUERYRESULT, PCODE_MLD_QUERYRESULT and
 * PCODE_MLD_QUERYRESULT2 packets given the correct set of
 * features. The features present in the 'old' query result packets
 * are defined in this enum along with the Query Result Features
 * themselves. The value called QRF_SUPPORTED_MASK denotes which
 * features are supported by the current version. If a packet with
 * unknown features is received on the network is is discarded (as it
 * would be if it had an illegal PCODE).
 **/
enum queryresult_features {
    QRF_MLD                   = 0x00000001,
    QRF_SORTDATA              = 0x00000010,
    QRF_AGGRDATA              = 0x00000020,
    QRF_COVERAGE              = 0x00000040,
    QRF_GROUPDATA             = 0x00000200,
    QRF_PROPERTIES            = 0x00000400,

    QRF_QUERYRESULT_MASK      = 0,
    QRF_MLD_QUERYRESULT_MASK  = QRF_MLD
};


/**
 * The new PCODE_QUERYX packet contains a 32-bit field called
 * 'featureflags'. Each bit in that field denotes a separate feature
 * that may be present in the query packet or not. The comment
 * describing the packet format indicates what data fields depend on
 * what features. Note that after removing the query id and the
 * feature flags from a PCODE_QUERYX packet it is binary compatible
 * with the PCODE_PARSEDQUERY2 packets
 * given the correct set of features. The features present in the
 * 'old' query packets are defined in this enum along with the Query
 * Features themselves. The values called
 * QF_SUPPORTED_[FSEARCH/FDISPATCH]_MASK denotes which features are
 * supported by the current version. If a packet with unknown features
 * is received on the network is is discarded (as it would be if it
 * had an illegal PCODE).
 **/
enum query_features {
    QF_PARSEDQUERY            = 0x00000002,
    QF_RANKP                  = 0x00000004,
    QF_SORTSPEC               = 0x00000080,
    QF_AGGRSPEC               = 0x00000100,
    QF_LOCATION               = 0x00000800,
    QF_PROPERTIES             = 0x00100000,
    QF_WARMUP		      = 0x00200000, // Deprecated, do not use!
    QF_GROUPSPEC              = 0x00400000,
    QF_SESSIONID              = 0x00800000,

    QF_PARSEDQUERY2_MASK      = (QF_PARSEDQUERY | QF_RANKP)
};


/**
 * The new PCODE_GETDOCSUMSX packet contains a 32-bit field called
 * 'featureflags'. Each bit in that field denotes a separate feature
 * that may be present in the getdocsums packet or not. The comment
 * describing the packet format indicates what data fields depend on
 * what features. Note that after removing the query id and the
 * feature flags from a PCODE_GETDOCSUMSX packet it is binary
 * compatible with the PCODE_GETDOCSUMS, PCODE_MLD_GETDOCSUMS and
 * PCODE_MLD_GETDOCSUMS2 packets given the correct set of
 * features. The features present in the 'old' getdocsums packets are
 * defined in this enum along with the GetDocsums Features
 * themselves. The values called
 * GDF_SUPPORTED_[FSEARCH/FDISPATCH]_MASK denotes which features are
 * supported by the current version. If a packet with unknown features
 * is received on the network is is discarded (as it would be if it
 * had an illegal PCODE).
 **/
enum getdocsums_features {
    GDF_MLD                   = 0x00000001,
    GDF_QUERYSTACK            = 0x00000004,
    GDF_RANKP_QFLAGS          = 0x00000010,
    GDF_LOCATION              = 0x00000080,
    GDF_RESCLASSNAME          = 0x00000800,
    GDF_PROPERTIES            = 0x00001000,
    GDF_FLAGS                 = 0x00002000,

    GDF_GETDOCSUMS_MASK       = 0,
    GDF_MLD_GETDOCSUMS_MASK   = (GDF_MLD)
};


enum getdocsums_flags
{
    GDFLAG_IGNORE_ROW         = 0x00000001,
    GDFLAG_ALLOW_SLIME        = 0x00000002
};

// docsum class for slime tunneling
const uint32_t SLIME_MAGIC_ID = 0x55555555;

enum monitorquery_features
{
    MQF_QFLAGS		    = 0x00000002,			

    MQF_MONITORQUERY_MASK   = 0
};


enum monitorquery_flags
{
    // NOT_USED MQFLAG_REPORT_SOFTOFFLINE = 0x00000010,
    MQFLAG_REPORT_ACTIVEDOCS  = 0x00000020
};


enum monitorresult_features
{
    MRF_MLD		    = 0x00000001,
    MRF_RFLAGS		    = 0x00000008,
    MRF_ACTIVEDOCS	    = 0x00000010,

    MRF_MONITORRESULT_MASK  = 0,
    MRF_MLD_MONITORRESULT_MASK = (MRF_MLD)
};


enum monitorresult_flags
{
    // NOT_USED MRFLAG_SOFTOFFLINE       = 0x00000001
};


/**
 * Codes for packets between dispatch nodes and search nodes.
 * general packet (i.e. message) format:
 * uint32_t  packetLength- length in bytes, EXCLUDING this length field
 * packetcode pCode	- see the enum below; same length as uint32_t
 * packetData		- variable length
 */
enum packetcode {
    PCODE_EOL = 200,	/* ..fdispatch <-> ..fsearch.	PacketData:
			 *0	{uint32_t queryId,}	- only in new format!*/
    PCODE_QUERY_NOTUSED = 201,
    PCODE_QUERYRESULT = 202,	/* ..fdispatch <-  ..fsearch.	PacketData:
			 *0	{uint32_t queryId,}	- only in new format!
			 *1	uint32_t offset,
			 *2	uint32_t numDocs,
			 *3	uint32_t totNumDocs,
			 *4	search::HitRank maxRank,
			 *5	time_t docstamp,	- sent as Uint32
			 *6	struct FastS_connhitresult {
			 *	    uint32_t docid;
			 *	    search::HitRank metric
			 *	}[] hits				     */
    PCODE_ERROR = 203,          /* ..fdispatch <-  ..fsearch/..fdispatch
                           *	{uint32_t queryId,}	- only in new format!
                           *      uint32_t  error_code  [see common/errorcodes.h]
                           *      uint32_t  message_len
                           *      char[]    message     (UTF-8)   */
    PCODE_GETDOCSUMS = 204,	/* ..fdispatch  -> ..fsearch.	PacketData:
			 *0	{uint32_t queryId,}	- only in new format!
			 *	time_t docstamp		- header
			 * 	uint32_t[] docid	- body	  	   */
    PCODE_DOCSUM = 205,		/* ..fdispatch <-  ..fsearch.
                                 *0	{uint32_t queryId,}	- only in new format!
                                 *1	uint32_t location
                                 *2	char[] <title, incipit, URL, ...>
                                 */
    PCODE_MONITORQUERY = 206,	/* ..fdispatch  -> ..fsearch.	No packet data.
			 */
    PCODE_MONITORRESULT = 207,	/* ..fdispatch <-  ..fsearch.	PacketData:
                                 *	int partitionId,
                                 *	time_t timeStamp			     */
    PCODE_MLD_QUERYRESULT = 208,/* ..fdispatch <-  ..fdispatch.
                           * header: {queryId,} offset, numdocs, tnumdocs,
                           *	maxRank, docstamp
                           * body: (docid, metric, partition, docstamp)*
                           */
    PCODE_MLD_GETDOCSUMS = 209,	/* ..fdispatch  -> ..fdispatch.
                                 * header: {queryId,} docstamp
                                 * body: (docid, partition, docstamp)*
                                 */
    PCODE_MLD_MONITORRESULT = 210 ,/* ..fdispatch <- ..fdispatch	NB: no queryId!
                             *	lowest partition id,
                             *	timestamp,
                             *	total number of nodes,
                             *	active nodes,
                             *	total number of partitions,
                             *	active partitions
                             */
    PCODE_CLEARCACHES = 211,	/* ..fdispatch -> ..fdispatch.  No packet data/ NotUsed
			 */
    PCODE_QUERY2_NOTUSED = 212,
    PCODE_PARSEDQUERY2 = 213,	/* ..fdispatch  -> ..fsearch.	PacketData:
			 *0	{uint32_t queryId,}	- only in new format!
			 *1	..query::querytypes searchType,	- all/any/exact
			 *2	uint32_t offset,
			 *3	uint32_t maxhits,
			 *4	uint32_t qflags,	(including usehardware)
			 *5     uint32_t rankprofile,   - enum
			 *6     uint32_t numStackItems,
			 *7	multiple encoded stackitems:
                         - uint32_t OR|AND|NOT|RANK
                         uint32_t arity
                         - uint32_t PHRASE
                         uint32_t arity
                         uint32_t indexNameLen
                         char[]   indexName
                         - uint32_t TERM
                         uint32_t indexNameLen
                         char[]   indexName
                         uint32_t termLen
                         char[]   term
                        */
    PCODE_MLD_QUERYRESULT2_NOTUSED = 214,
    PCODE_MLD_GETDOCSUMS2_NOTUSED = 215,

    PCODE_QUEUELEN = 216,	/* fdispatch <- fsearch.
			 * header: queueLen, dispatchers
			 */
    PCODE_QUERYRESULTX = 217,	/*
			 *      {uint32_t queryId,}    - only if persistent
                         *      uint32_t featureflags, - see 'queryresult_features'
			 *      uint32_t offset,
			 *      uint32_t numDocs,
			 *      uint32_t totNumDocs,
			 *      search::HitRank maxRank,
			 *      uint32_t docstamp,
                         *      uint32_t[numDocs] sortIndex   - if QRF_SORTDATA
                         *      char[sidx[n - 1]] sortData    - if QRF_SORTDATA
                         *      uint32_t          aggrDataLen - if QRF_AGGRDATA
                         *      char[aggrDataLen] aggrData    - if QRF_AGGRDATA
                         *      uint32_t           groupDataLen - if QRF_GROUPDATA
                         *      char[groupDataLen] groupData    - if QRF_GROUPDATA
                         *      uint64_t coverageDocs  - if QRF_COVERAGE
                         *      uint32_t coverageNodes - if QRF_COVERAGE
                         *      uint32_t coverageFull  - if QRF_COVERAGE
			 *      numDocs * hit {
			 *	    uint32_t docid,
			 *	    search::HitRank metric,
                         *          uint32_t partid,   - if QRF_MLD
                         *          uint32_t docstamp, - if QRF_MLD
			 *	}			     */
    PCODE_QUERYX = 218,	        /*
                                 * 	{uint32_t queryId,}          - only if persistent
                                 *      uint32_t featureflags,       - see 'query_features'
                                 * 	uint32_t querytype
                                 * 	uint32_t offset,
                                 * 	uint32_t maxhits,
                                 * 	uint32_t qflags,
                                 *      uint32_t minhits,            - if QF_MINHITS
                                 *      uint32_t numProperties       - if QF_PROPERTIES
                                 *      numProperties * props {      - if QF_PROPERTIES
                                 *        uint32_t nameLen
                                 *        char[nameLen] name
                                 *        uint32_t numEntries
                                 *        numentries * entry {
                                 *          uint32_t keyLen
                                 *          char[keyLen] key
                                 *          uint32_t valueLen
                                 *          char[valueLen] value
                                 *        }
                                 *      }
                                 *      uint32_t sortSpecLen         - if QF_SORTSPEC
                                 *      char[sortSpecLen] sortSpec   - if QF_SORTSPEC
                                 *      uint32_t aggrSpecLen         - if QF_AGGRSPEC
                                 *      char[aggrSpecLen] aggrSpec   - if QF_AGGRSPEC
                                 *      uint32_t groupSpecLen         - if QF_GROUPSPEC
                                 *      char[groupSpecLen] groupSpec  - if QF_GROUPSPEC
                                 *      uint32_t locationLen         - if QF_LOCATION
                                 *      char[locationLen] location   - if QF_LOCATION
                                 *      uint32_t numStackItems,      - if QF_PARSEDQUERY
                                 *	multiple encoded stackitems: - if QF_PARSEDQUERY
                                 - uint32_t OR|AND|NOT|RANK
                                 uint32_t arity
                                 - uint32_t PHRASE
                                 uint32_t arity
                                 uint32_t indexNameLen
                                 char[]   indexName
                                 - uint32_t TERM
                                 uint32_t indexNameLen
                                 char[]   indexName
                                 uint32_t termLen
                                 char[]   term
                                 */
    PCODE_GETDOCSUMSX = 219,	/*
			 *      {uint32_t queryId,}           - only if persistent
                         *      uint32_t featureflags,        - see 'getdocsums_features'
			 *      uint32_t docstamp,
                         *      uint32_t rankprofile,         - if GDF_RANKP_QFLAGS
                         *      uint32_t qflags,              - if GDF_RANKP_QFLAGS
                         *      uint32_t resClassNameLen      - if GDF_RESCLASSNAME
                         *      char []  resClassName         - if GDF_RESCLASSNAME
                         *      uint32_t numProperties        - if GDF_PROPERTIES
                         *      numProperties * props {       - if GDF_PROPERTIES
                         *        uint32_t nameLen
                         *        char[nameLen] name
                         *        uint32_t numEntries
                         *        numentries * entry {
                         *          uint32_t keyLen
                         *          char[keyLen] key
                         *          uint32_t valueLen
                         *          char[valueLen] value
                         *        }
                         *      }
			 *      uint32_t stackItems,          - if GDF_STACKDUMP
			 *      uint32_t stackDumpLen,        - if GDF_STACKDUMP
			 *      char[stackDumpLen] stackDump, - if GDF_STACKDUMP
                         *      uint32_t locationLen          - if GDF_LOCATION
                         *      char[locationLen] location    - if GDF_LOCATION
			 *      N * doc {
			 *          uint32_t docid,
			 *          uint32_t partid,          - if GDF_MLD
			 *          uint32_t docstamp,        - if GDF_MLD
                         *      }
                         */
    PCODE_MONITORQUERYX = 220,  /*
                           *	uint32_t featureFlags;
                           *		- see monitorquery_features
                           */
    PCODE_MONITORRESULTX = 221, /*
                           *	uint32_t featureFlags;
                           *		- see monitorresult_features
                           *	uint32_t partitionId;
                           *	uint32_t timestamp;
                           *	uint32_t totalNodes;	      - if MRF_MLD
                           *	uint32_t activeNodes;	      - if MRF_MLD
                           *	uint32_t totalParts;	      - if MRF_MLD
                           *	uint32_t activeParts;	      - if MRF_MLD
                           */
    PCODE_TRACEREPLY = 222,  /*
                         *      numProperties * props {
                         *        uint32_t nameLen
                         *        char[nameLen] name
                         *        uint32_t numEntries
                         *        numentries * entry {
                         *          uint32_t keyLen
                         *          char[keyLen] key
                         *          uint32_t valueLen
                         *          char[valueLen] value
                         *        }
                         *      }
                         */
    PCODE_LastCode = 223	// Used for consistency checking only, must be last.
};

} // namespace fs4transport
} // namespace search

