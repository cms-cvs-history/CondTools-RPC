#ifndef RPCEMAPSOURCEHANDLER
#define RPCEMAPSOURCEHANDLER

#include <vector>
#include <string>
#include <iostream>
#include <typeinfo>

#include "OnlineDB/Oracle/interface/Oracle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "CondCore/PopCon/interface/PopConSourceHandler.h"
//#include "CondCore/PopCon/interface/LogReader.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSetfwd.h"

#include "CondFormats/RPCObjects/interface/RPCEMap.h"
#include "CondFormats/DataRecord/interface/RPCEMapRcd.h"
#include "CondFormats/RPCObjects/interface/RPCReadOutMapping.h"
#include "CondFormats/RPCObjects/interface/DccSpec.h"
#include "CondFormats/RPCObjects/interface/TriggerBoardSpec.h"
#include "CondFormats/RPCObjects/interface/LinkConnSpec.h"
#include "CondFormats/RPCObjects/interface/LinkBoardSpec.h"
#include "CondFormats/RPCObjects/interface/ChamberLocationSpec.h"
#include "CondFormats/RPCObjects/interface/FebLocationSpec.h"
#include "CondFormats/RPCObjects/interface/FebConnectorSpec.h"
#include "CondFormats/RPCObjects/interface/ChamberStripSpec.h"
#include "CondFormats/RPCObjects/interface/DBSpecToDetUnit.h"

#include "CondCore/DBCommon/interface/SessionConfiguration.h"
#include "CondCore/DBCommon/interface/ConnectionConfiguration.h"
#include "CondCore/DBCommon/interface/DbTransaction.h"
#include "CondCore/DBCommon/interface/DbSession.h"
#include "CondCore/DBCommon/interface/DbConnection.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"

using namespace std;
using namespace oracle::occi;

namespace popcon
{
	class RPCEMapSourceHandler : public popcon::PopConSourceHandler<RPCEMap>
	{

		public:
    RPCEMapSourceHandler(const edm::ParameterSet& ps);
    ~RPCEMapSourceHandler();
    void getNewObjects();
    std::string id() const {return m_name;}
    void ConnectOnlineDB(string connect, string authPath);
    void DisconnectOnlineDB();
    void readEMap1();
    int Compare2EMaps(Ref map1, RPCEMap* map2);

		private:
    RPCEMap * eMap;
    Connection* conn;
    cond::DbSession * session;
    cond::DbConnection * connection ;
    std::string m_name;
    int m_dummy;
    int m_validate;
    std::string m_connect;
    std::string m_authpath;

  // utilities
    string IntToString(int num)
    {
      stringstream snum;
      snum << num << flush;
      return(snum.str());
    }

    typedef struct{int febId,chamberId,connectorId,lbInputNum,posInLocalEtaPart,posInCmsEtaPart;string localEtaPart,cmsEtaPart;} FEBStruct;
	};
}
#endif
