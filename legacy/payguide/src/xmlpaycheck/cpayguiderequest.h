// 
// File:   cpayguiderequest.h
// Author: alex
//
// Created on 14 Март 2008 г., 3:25
//

#ifndef _CPAYGUIDEREQUEST_H
#define	_CPAYGUIDEREQUEST_H

#include <string>
#include "../../lib/libbonbon/cjob.h"
#include "csocket.h"

#define REQUEST_MAX_SIZE 2048

namespace paycheck
{
    class CPayguideRequest: public bonbon::CJob
    {
//	static bool CheckSign(std::string &bill_num, std::string &data, std::string &provider, std::string &terminal, std::string &currency, std::string &login, std::string &checksum, std::string &sign);
	public:
	    //CPayguideRequest(std::string &bill_num, std::string &data, std::string &provider, std::string &terminal, std::string &currency, std::string &login, std::string &checksum, paycheck::CSocket &soc, char t);
	    CPayguideRequest(paycheck::CSocket &sock);
            ~CPayguideRequest();
//            bool CheckSign(std::string &sign);
	    //int Compile(char *buff, int buff_size);
            paycheck::CSocket *GetSocket();

        protected:
            friend class CPaycheckCore;
//	    std::string check_sum;
//	    std::string session;
//	    std::string data;
//	    size_t data_size;
//	    bool bin_data;
//	    std::string provider;
//	    std::string terminal;
//	    std::string currency;
//	    std::string login;
	    paycheck::CSocket *socket;
//            std::string signedstr;
//	    char type;
//	    unsigned int magic;
            char bin[REQUEST_MAX_SIZE];
            int bin_size;
    };
}

#endif	/* _CPAYGUIDEREQUEST_H */

