#ifndef RPC_DB_FW_H
#define RPC_DB_FW_H

/*
 * \class RPCFw
 *  Reads data from OMDS and creates conditioning objects
 *
 *  $Date: 2008/10/11 08:46:50 $
 *  $Revision: 1.4 $
 *  \author D. Pagano - Dip. Fis. Nucl. e Teo. & INFN Pavia
 */



#include "CondTools/RPC/interface/RPCDBCom.h"
#include "CoralBase/TimeStamp.h"
<<<<<<< RPCFw.h
#include "CondTools/RPC/interface/RPCImonSH.h"
#include "CondTools/RPC/interface/RPCVmonSH.h"
#include "CondTools/RPC/interface/RPCStatusSH.h"
#include "CondTools/RPC/interface/RPCTempSH.h"

=======
#include "CondTools/RPC/interface/RPCCondSH.h"
>>>>>>> 1.4
#include "CondTools/RPC/interface/RPCGasSH.h"
#include "CondTools/RPC/interface/RPCIDMapSH.h"


struct dbread{
    float alias;
    float value;
};


class RPCFw : virtual public RPCDBCom
{
public:
  RPCFw( const std::string& connectionString,
         const std::string& userName,
         const std::string& password);
  virtual ~RPCFw();
  void run();

  coral::TimeStamp UTtoT(int utime);


  coral::TimeStamp thr;
<<<<<<< RPCFw.h
  std::vector<RPCObImon::I_Item> createIMON(int from);
  std::vector<RPCObVmon::V_Item> createVMON(int from); 
  std::vector<RPCObStatus::S_Item> createSTATUS(int from); 
  std::vector<RPCObGas::Item> createGAS(int from);
  std::vector<RPCObTemp::T_Item> createT(int from);
  std::vector<RPCObPVSSmap::Item> createIDMAP();

=======
  std::vector<RPCObCond::Item> createIMON(int from);
  std::vector<RPCObCond::Item> createVMON(int from); 
  std::vector<RPCObCond::Item> createSTATUS(int from); 
  std::vector<RPCObGas::Item> createGAS(int from);
  std::vector<RPCObCond::Item> createT(int from);
>>>>>>> 1.4
  
private:
  std::string m_connectionString;
  std::string m_userName;
  std::string m_password;
};

#endif
