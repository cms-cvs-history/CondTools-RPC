import FWCore.ParameterSet.Config as cms

process = cms.Process("Write2DB")
process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.CondDBCommon.connect = 'sqlite_file:L1RPCHwConfig.db'

process.MessageLogger = cms.Service("MessageLogger",
    cout = cms.untracked.PSet(
        default = cms.untracked.PSet(
            limit = cms.untracked.int32(0)
        )
    ),
    destinations = cms.untracked.vstring('cout')
)

process.source = cms.Source("EmptyIOVSource",
    firstValue = cms.uint64(0),
    lastValue = cms.uint64(0),
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
        WriteDummy = cms.untracked.int32(0),
        Validate = cms.untracked.int32(0),
        OnlineConn = cms.untracked.string('oracle://cms_omds_lb/CMS_RPC_CONF')
        OnlineAuthPath = cms.untracked.string('.'),
    )
)

process.p = cms.Path(process.WriteInDB)
