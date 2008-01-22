
/** \class RPCReadOutMapingO2O
 *
 * Description:
 *      Class to read directly OMDS DB with OCCI and fill Offline DB
 *
 * $Date: $
 * $Revision: $
 * \author Michal Bluj -- INS Warsaw
 *
 */

#include "CondFormats/RPCObjects/interface/RPCReadOutMapping.h"
#include "CondFormats/RPCObjects/interface/DccSpec.h"
#include "CondFormats/RPCObjects/interface/TriggerBoardSpec.h"
#include "CondFormats/RPCObjects/interface/LinkConnSpec.h"
#include "CondFormats/RPCObjects/interface/LinkBoardSpec.h"
#include "CondFormats/RPCObjects/interface/ChamberLocationSpec.h"
#include "CondFormats/RPCObjects/interface/FebSpec.h"
#include "CondFormats/RPCObjects/interface/ChamberStripSpec.h"

//#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include "CondCore/DBCommon/interface/DBWriter.h"
#include "CondCore/DBCommon/interface/DBSession.h"
#include "CondCore/DBCommon/interface/Exception.h"
#include "CondCore/DBCommon/interface/ServiceLoader.h"
#include "CondCore/DBCommon/interface/ConnectMode.h"
#include "CondCore/DBCommon/interface/MessageLevel.h"
#include "CondCore/IOVService/interface/IOV.h"
#include "CondCore/MetaDataService/interface/MetaData.h"

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "occi.h"

using namespace std;
using namespace oracle::occi;

class RPCReadOutMapingO2O {
  
public:

  RPCReadOutMapingO2O(string host, string sid, string user, string pass, int port=1521, string poolDb="sqlite_file:cabling.db", string version="RPCReadOutMapping_v1", int first_run=1, int last_run=1) 
  { 
    ConnectOnlineDB(host, sid, user, pass, port);
    poolDbCon_=poolDb;

    version_=version;
    catalog_="file:cablingCatalog.xml";
    
    first_run>0 ? run1_=first_run : run1_=1;
    last_run>run1_ ? run2_=last_run : run2_=run1_;

    ::putenv("CORAL_AUTH_USER=konec");
    ::putenv("CORAL_AUTH_PASSWORD=konecPass"); 

  }
 
 ~RPCReadOutMapingO2O()
  { 
    DisconnectOnlineDD();
    WritePoolDb();
  }

  void ConnectOnlineDB(string host, string sid, string user, string pass, int port=1521)
  {
    stringstream ss;
    ss << "//" << host << ":" << port << "/" << sid;
    
    cout << "Connecting to " << host << "..." << flush;
    env = Environment::createEnvironment(Environment::OBJECT);
    conn = env->createConnection(user, pass, ss.str());
    cout << "Done." << endl;
  }

  void DisconnectOnlineDD()
  {
    env->terminateConnection(conn);
    Environment::terminateEnvironment(env);
  }

  void WritePoolDb()
  {
    cout << endl << "Start writing to PoolDB" << flush << endl;
    try {
      loader = new cond::ServiceLoader;
      loader->loadAuthenticationService( cond::Env );
      loader->loadMessageService( cond::Error );

      session = new cond::DBSession(poolDbCon_);
      session->setCatalog(catalog_);
      session->connect(cond::ReadWriteCreate);

      metadataSvc = new cond::MetaData(poolDbCon_,*loader);
      metadataSvc->connect();

      mapWriter = new cond::DBWriter(*session, "RPCReadOutMapping");
      iovWriter = new cond::DBWriter(*session, "IOV");

      string cabTok;
      string cabIOVTok;
      cond::IOV* cabIOV= new cond::IOV; 

      // Write same mapping for all runs 
      for(int run=run1_; run<=run2_;run++){
	session->startUpdateTransaction();
        cout << " Run " << run << endl << "  Write cabling..." << flush;
	cabTok = mapWriter->markWrite<RPCReadOutMapping>(cabling);  
	//cout << " Commit... " << flush;
	//session->commit();
	cout << " Done." << endl;
	cout << "  Associate IOV..." << flush;
	cabIOV->iov.insert(std::make_pair(run, cabTok));
	cout << " Done." << endl;
      }
      session->startUpdateTransaction();

      cout << " Write IOV... " << flush;
      cabIOVTok = iovWriter->markWrite<cond::IOV>(cabIOV);  // ownership given
      cout << " Commit... " << flush;
      session->commit();  // IOV memory freed
      cout << " Done." << endl;

      cout << " Add MetaData..." << flush;
      metadataSvc->addMapping(version_,cabIOVTok);
      cout << " Done." << endl;

      session->disconnect();
      metadataSvc->disconnect();
    }
    catch(const cond::Exception& e) {
      std::cout<<"cond::Exception: " << e.what()<<std::endl;
      if(loader) delete loader;
    } 
    catch (pool::Exception& e) {
      cout << "pool::Exception:  " << e.what() << endl;
      if(loader) delete loader;
    }
    catch (std::exception &e) {
      cout << "std::exception:  " << e.what() << endl;
      if(loader) delete loader;
    }
    catch (...) {
      cout << "Unknown error caught" << endl;
      if(loader) delete loader;
    }
    if(session) delete session;
    if (mapWriter) delete mapWriter;
    if(iovWriter) delete iovWriter;
    if (metadataSvc) delete metadataSvc;
    if(loader) delete loader;
    cout << "PoolDB written!" << endl;
  }
  

// Method called to read Online DB and fill RPCReadOutMapping
  void readCablingMap()
  {

    cout << "Reading cabling map with mode 0:  Basic OCCI" << endl;

    string cabling_version = version_;

    Statement* stmt = conn->createStatement();
    string sqlQuery ="";

    cout << endl <<"Start to build RPC Cabling..." << flush << endl << endl;
    cabling =  new RPCReadOutMapping(cabling_version);

    // Get FEDs
    cout << "Getting the FEDs... " << flush;
    sqlQuery=" SELECT DCCBoardId, DAQId FROM DCCBoard WHERE DCCBoardId>0 ORDER BY DAQId ";
    stmt->setSQL(sqlQuery.c_str());
    ResultSet* rset = stmt->executeQuery();
    std::pair<int,int> tmp_tbl;
    std::vector< std::pair<int,int> > theDAQ;
    while (rset->next()) {
      tmp_tbl.first=rset->getInt(1);
      tmp_tbl.second=rset->getInt(2);
      theDAQ.push_back(tmp_tbl);
    }
    cout << "Done." << endl;
    for(int iFed=0;iFed<theDAQ.size();iFed++) {
      cout << " |-> Getting the TBs for FED no. "<< theDAQ[iFed].second << " ... " << flush;
      std::vector<std::pair<int,int> > theTB;
      DccSpec dcc(theDAQ[iFed].second);
      sqlQuery = " SELECT TriggerBoardId, DCCInputChannelNum FROM TriggerBoard ";
      sqlQuery += " WHERE DCCBoard_DCCBoardId= ";
      sqlQuery += IntToString(theDAQ[iFed].first);
      sqlQuery += " ORDER BY DCCInputChannelNum ";
      stmt->setSQL(sqlQuery.c_str());
      rset = stmt->executeQuery();
      while (rset->next()) {
	tmp_tbl.first=rset->getInt(1);
	tmp_tbl.second=rset->getInt(2);
	theTB.push_back(tmp_tbl);
      }
      cout << "Done." << endl;
      for(int iTB=0;iTB<theTB.size();iTB++) {
	cout << " |    |-> Getting the Links for TB no. "<< theTB[iTB].second << " ... " << flush;
	std::vector<std::pair<int,int> > theLink;
	TriggerBoardSpec tb(theTB[iTB].second);
	sqlQuery = " SELECT Board_BoardId, TriggerBoardInputNum FROM LinkConn ";
	sqlQuery += " WHERE TB_TriggerBoardId= ";
	sqlQuery +=  IntToString(theTB[iFed].first);  
	sqlQuery += " ORDER BY TriggerBoardInputNum ";
	stmt->setSQL(sqlQuery.c_str());
	rset = stmt->executeQuery();
	while (rset->next()) {
	  tmp_tbl.first=rset->getInt(1);
	  tmp_tbl.second=rset->getInt(2);
	  theLink.push_back(tmp_tbl);
	}
	cout << "Done." << endl;
	for(int iLink=0;iLink<theLink.size();iLink++) {
	  cout << " |    |    |-> Getting the LBs for Link no. "<< theLink[iLink].second << " ... " << flush;
	  //
	  // FIXME: dummy mapping for LBs, chambers, FEBs and strips
	  // 
	  /*
	    typedef struct{ bool m, int id, ch;} link_struct; 
	    std::vector<link_struct> theLB;
	    LinkConnSpec lc(theLink[iLink].second);
	    //	  sqlQuery = " SELECT nvl ( MasterId, -1 ) FROM LinkBoard ";
	    //	  sqlQuery += " WHERE LinkBoardId = ";
	    //	  sqlQuery += 
	    sqlQuery = " SELECT name FROM Board ";
	    sqlQuery += " WHERE BoardId =  ";
	    sqlQuery += 
	    stmt->setSQL(sqlQuery.c_str());
	    rset = stmt->executeQuery();
	    while (rset->next()) {
	    bool master = false;
	    int master_id = rset->getInt(1);
	    if (master_id == -1 || master_id == theLink[iLink].first) { 
	    master = true;
	    }
	    theLB.push_back(tmp_tbl);
	  */
	  LinkConnSpec  lc(iLink); 
	  cout << "Done." << endl;
	  for (int iLB=0; iLB <=2; iLB++) {
	    bool master = (iLB==0);
	    ChamberLocationSpec chamber = {1,5,3,"+","ch","IN","+z","Barrel"};
	    LinkBoardSpec lb(master, iLB, chamber);
	    for (int iFEB=0; iFEB <= 5; iFEB++) {
	      FebSpec feb(iFEB,"F",2);
	      for (int iStrip=0; iStrip <= 15; iStrip++) {
		ChamberStripSpec strip = {iStrip, iFEB*16+iStrip};
		feb.add(strip);
	      }
	      lb.add(feb); 
	    }
	    lc.add(lb);
	  }
	  tb.add(lc);
	}
	dcc.add(tb);
      }
      cabling->add(dcc);
    }
    cout << endl <<"Building RPC Cabling done!" << flush << endl << endl;
  }
  
private:
  Environment* env;
  Connection* conn;

  cond::ServiceLoader* loader;
  cond::DBSession* session;
  cond::DBWriter* mapWriter;
  cond::DBWriter* iovWriter;
  cond::MetaData* metadataSvc;

  string poolDbCon_;
  
  RPCReadOutMapping *cabling;

  string version_;
  string catalog_;
  int run1_, run2_;

  // utilities
  string IntToString(int num)
  {
    stringstream snum;
    snum << num << flush;

    return(snum.str());
  }
};

int main(int argc, char* argv[])
{

  // simple parsing command-line options
  if( (argc < 4 && argc!=2) || (argc==2 && (string)argv[1]!="-help") ) {
    cout << "Usage:" << endl;
    cout << "  " << argv[0] << " <host> <SID> <user> <passwd> "
	 << "[port] [POOL_DB] [version] [first_run] [last_run]" 
	 << endl;
    cout << "Try \'" << argv[0] << " -help\' for more information." << endl;
    return -1;
  }
  if (argc==2 && (string)argv[1]=="-help") {
    cout << "Usage:" << endl;
    cout << "  " << argv[0] << " <host> <SID> <user> <passwd> "
	 << "[port] [POOL DB] [version] [first run] [last run]" 
	 << endl;
    cout << "where:" << endl
	 << "  host, ID, user, passwd, port identify Online DB (default: port=1521)" << endl
	 << "  POOL_DB is POOL DB connection string (default: sqlite_file:cabling.db)" << endl
	 << "  version is a tag of Mapping (default: RPCReadOutMapping_v1)" << endl
	 << "  first_run, last_run are first and last runs for which Mapping is valid (default: first_run=last_run=1)" << endl
	 << "Warning:" << endl
	 << "  Positions of parameters cannot be changed!" << endl
	 << endl;
    return 1;
  }
  string host = (string)argv[1];
  string sid = (string)argv[2];
  string user = (string)argv[3];
  string pass = (string)argv[4];
  int port = 1521;
  string poolCon = "sqlite_file:cabling.db";
  string ver = "RPCReadOutMapping_v1";
  int run1=1, run2=1;
  if(argc > 5 ) port = atoi(argv[5]);
  if(argc > 6 ) poolCon = (string)argv[6];
  if(argc > 7 ) ver = (string)argv[7];
  if(argc > 8 ) run1 = atoi(argv[8]);
  if(argc > 9 ) run2 = atoi(argv[9]);

  try {
    RPCReadOutMapingO2O o2o(host, sid, user, pass, port, poolCon, ver, run1, run2);
    
    o2o.readCablingMap();
      
  } catch (SQLException &e) {
    cerr << "SQLException:  " << e.getMessage() << endl;
  } catch (exception &e) {
    cerr << "exception:  " << e.what() << endl;
  } catch (...) {
    cerr << "Unknown exception caught" << endl;
  }

  return 0;
}
