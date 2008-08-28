/*
 *  See headers for a description
 *
 *  $Date: 2008/07/31 14:48:25 $
 *  $Revision: 1.6 $
 *  \author D. Pagano - Dip. Fis. Nucl. e Teo. & INFN Pavia
 */

#include "CondTools/RPC/interface/RPCGasSH.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "FWCore/ParameterSet/interface/ParameterSetfwd.h"

#include<iostream>

popcon::RpcGasTData::RpcGasTData(const edm::ParameterSet& pset) :
  m_name(pset.getUntrackedParameter<std::string>("name","RpcData")),
  host(pset.getUntrackedParameter<std::string>("host", "source db host")),
  user(pset.getUntrackedParameter<std::string>("user", "source username")),
  passw(pset.getUntrackedParameter<std::string>("passw", "source password")),
  m_since(pset.getUntrackedParameter<unsigned long long>("since",5)){
}

popcon::RpcGasTData::~RpcGasTData()
{
}

void popcon::RpcGasTData::getNewObjects() {

  std::cout << "------- " << m_name << " - > getNewObjects\n" 
	    << "got offlineInfo"<< tagInfo().name 
	    << ", size " << tagInfo().size << ", last object valid since " 
	    << tagInfo().lastInterval.first << " token "   
            << tagInfo().lastPayloadToken << std::endl;

  std::cout << " ------ last entry info regarding the payload (if existing): " 
	    << logDBEntry().usertext << "last record with the correct tag has been written in the db: "
	    << logDBEntry().destinationDB << std::endl; 
  
  snc = tagInfo().lastInterval.first;

  //--------------------------IOV
  std::string str;
  time_t t;
  t = time (NULL);
  std::stringstream ss;
  ss << t; ss >> str;
  std::cout << "Now ==> UNIX TIME = " << str << std::endl;
  utime = atoi (str.c_str());  
  //-----------------------------


  if (snc > 0) { niov = utime;} else { snc = m_since; niov = utime; }

  std::cout << "New IOV: since is = " << niov << std::endl;

  
  RPCFw caen ( host, user, passw );

  std::vector<RPCGasT::GasItem> Gascheck;
  std::vector<RPCGasT::TempItem> Tcheck;


  Gascheck = caen.createGAS(snc);
  Tcheck = caen.createT(snc);  

  Gasdata = new RPCGasT();
  RPCGasT::GasItem Ifill;
  std::vector<RPCGasT::GasItem>::iterator Iit;
  for(Iit = Gascheck.begin(); Iit != Gascheck.end(); Iit++)
    {
      Ifill = *(Iit);
      Gasdata->Gas_rpc.push_back(Ifill);
    }
  std::cout << " >> Final object size: " << Gasdata->Gas_rpc.size() << std::endl;


  Tdata = new RPCGasT();
  RPCGasT::TempItem Tfill;
  std::vector<RPCGasT::TempItem>::iterator Tit;
  for(Tit = Tcheck.begin(); Tit != Tcheck.end(); Tit++)
    {
      Tfill = *(Tit);
      Tdata->Temp_rpc.push_back(Tfill);
    }
  std::cout << " >> Final object size: " << Tdata->Temp_rpc.size() << std::endl;


  m_to_transfer.push_back(std::make_pair((RPCGasT*)Gasdata,niov));
  m_to_transfer.push_back(std::make_pair((RPCGasT*)Tdata,niov+1));
}
