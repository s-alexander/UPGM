// 
// File:   cpaycheckcore.h
// Author: alex
//
// Created on 14 Март 2008 г., 3:28
//

#ifndef _CPAYCHECKCORE_H
#define	_CPAYCHECKCORE_H

#include <string>

#include "csocket.h"
#include "cconfig.h"
#define G_ERROR_START_PAGE 100
#define G_ERROR_QUEUE_FULL 110
#define G_ERROR_NO_ACTION 120
#define G_ERROR_ACTION_NOT_FOUND 130
#define G_ERROR_NOT_ALL_PARAMS 140
#define G_ERROR_ACTION_WRONG_OP 150
#define G_ERROR_WRONG_SIGN 160
#define G_ERROR_WRONG_USER 170
#define G_ERROR_WRONG_METHOD 180

class SPay;
class SPayResult;
namespace paycheck
{
    class CConfig;
    class CPaycheckCore
    {
	public:
	    CPaycheckCore();
	    ~CPaycheckCore();
	    int LoadConfig(const char *config_filename);
    	    int Xml2PayguideFormat(std::string &xml_txt, paycheck::CSocket &sock);
	    int GenericError(int error_code, paycheck::CSocket &socket);
	    int SimpleAnswer(std::string &content, paycheck::CSocket &socket);
	    SPay *XMLGetNextPay();
	    int XMLSetState(SPay *pay, SPayResult *payres);
	    const char *GetNetworkDevice();
	    long long int GetNetworkPort();
	protected:
	    paycheck::CConfig main_cfg;
	    paycheck::CConfig op_allow;
	    bool trusted_net;
	    std::queue<SPay *> pays;
	    bonbon::CMutex queue_lock;
	    

    };
    
}

#endif	/* _CPAYCHECKCORE_H */

