#!/bin/csh
#
eval `scramv1 runtime -csh`
set goon = 1
set period = 1800
cat >! writeL1RPCHwConfig2DB.inp << %


0
%
#
while ($goon == 1)
cat >! inspectRunInfo.py << %
import os,sys, DLFCN
sys.setdlopenflags(DLFCN.RTLD_GLOBAL+DLFCN.RTLD_LAZY)

from pluginCondDBPyInterface import *
a = FWIncantation()
rdbms = RDBMS("/afs/cern.ch/cms/DB/conddb")

dbName =  "oracle://cms_orcoff_prod/CMS_COND_31X_RUN_INFO"
logName = "oracle://cms_orcoff_prod/CMS_COND_31X_POPCONLOG"

from CondCore.Utilities import iovInspector as inspect

db = rdbms.getDB(dbName)
tags = db.allTags()
# for inspecting last run after run has started  
tag = 'runinfo_31X_hlt'

# for inspecting last run after run has stopped  
#tag = 'runinfo_test'

try :

    iov = inspect.Iov(db,tag,0,0,0,1)
    for x in  iov.summaries():
        print x[1]
except Exception, er :
    print "0"
%
#
set lastrun = `python inspectRunInfo.py`
if ( -f writeL1RPCHwConfig2DB.inp )then
 set disablecrates = `cat writeL1RPCHwConfig2DB.inp|head -1`
 set disabletowers = `cat writeL1RPCHwConfig2DB.inp|head -2|tail -1`
endif

cat >! writeL1RPCHwConfig2DB_cfg.py << %%
import FWCore.ParameterSet.Config as cms

process = cms.Process("Write2DB")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.CondDBCommon.connect = 'sqlite_file:L1RPCHwConfig.db'
#process.CondDBCommon.connect = 'oracle://cms_orcoff_prep/CMS_COND_31X_ALL'

process.MessageLogger = cms.Service("MessageLogger",
    cout = cms.untracked.PSet(
        default = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        )
    ),
    destinations = cms.untracked.vstring('cout')
)

process.source = cms.Source("EmptyIOVSource",
    firstValue = cms.uint64($lastrun),
    lastValue = cms.uint64($lastrun),
    timetype = cms.string('runnumber'),
    interval = cms.uint64(1)
)

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDBCommon,
    logconnect = cms.untracked.string('sqlite_file:L1RPCHwConfig_log.db'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('L1RPCHwConfigRcd'),
        tag = cms.string('L1RPCHwConfig_v1')
    ))
)

process.WriteInDB = cms.EDFilter("L1RPCHwConfigDBWriter",
    SinceAppendMode = cms.bool(True),
    record = cms.string('L1RPCHwConfigRcd'),
    loggingOn = cms.untracked.bool(True),
    Source = cms.PSet(
        FirstBX = cms.untracked.int32(0),
        LastBX = cms.untracked.int32(0),
#        DisabledCrates = cms.untracked.vint32(0,1,2,3,4,5),
        DisabledCrates = cms.untracked.vint32($disablecrates),
        DisabledTowers = cms.untracked.vint32($disabletowers),
        WriteDummy = cms.untracked.int32(1),
        Validate = cms.untracked.int32(0),
        OnlineAuthPath = cms.untracked.string('.'),
        OnlineConn = cms.untracked.string('oracle://cms_omds_lb/CMS_RPC_CONF')
    )
)

process.p = cms.Path(process.WriteInDB)
%%

cmsRun writeL1RPCHwConfig2DB_cfg.py
#
if ( -f writeL1RPCHwConfig2DB.inp )then
 set goon = `cat writeL1RPCHwConfig2DB.inp|tail -1`
endif
#
sleep $period
end
#
