#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <cctype>
#include <algorithm>

#include "cpayguiderequest.h"

//paycheck::CPayguideRequest::CPayguideRequest(std::string &bill, std::string &dat, std::string &prov, std::string &term, std::string &cur, std::string &log, std::string &check_s, paycheck::CSocket &soc, char t):bonbon::CJob()
paycheck::CPayguideRequest::CPayguideRequest(paycheck::CSocket &sock):bonbon::CJob()
{
//    socket=&soc;
//    session=bill;
//    data=dat;
//    provider=prov;
//    terminal=term;
//    currency=cur;
//    login=log;
//    check_sum=check_s;
//    bin_data=false;
//    data_size=data.length();
//    type=t;
    socket=&sock;
}

paycheck::CSocket *paycheck::CPayguideRequest::GetSocket()
{
    return socket;
}

paycheck::CPayguideRequest::~CPayguideRequest()
{
    
}

//bool paycheck::CPayguideRequest::CheckSign(std::string &sign)
//{
//    
//    std::string buff;
//    unsigned char md5digest[MD5_DIGEST_LENGTH];
//
//    MD5((unsigned char *)signedstr.c_str(),signedstr.length(), md5digest);
//
//    for (int i=0; i<MD5_DIGEST_LENGTH; i++) 
//    {
//        char tmps[8];
//        snprintf(tmps,9, "%02x",  md5digest[i]);
//        buff+=tmps;
//    }
//    if (buff==sign)
//        return true;
//    return false;
//}

//bool paycheck::CPayguideRequest::CheckSign(std::string &session, std::string &data, std::string &provider, std::string &terminal, std::string &currency, std::string &login, std::string &password, std::string &sign)
//{
//    std::string signedstr=session+data+provider+terminal+currency+login+password;
//    
//    std::transform(sign.begin(), sign.end(), sign.begin(), (int(*)(int)) std::toupper);
//
//    
//    std::string buff;
//    unsigned char md5digest[MD5_DIGEST_LENGTH];
//
//    MD5((unsigned char *)signedstr.c_str(),signedstr.length(), md5digest);
//
//    for (int i=0; i<MD5_DIGEST_LENGTH; i++) 
//    {
//        char tmps[8];
//        snprintf(tmps, 8, "%02x",  md5digest[i]);
//        buff+=tmps;
//    }
//    if (buff==sign)
//        return true;
//    return false;
//
//}
//
//int paycheck::CPayguideRequest::Compile(char *buff, int buff_size)
//{
//    if (buff==NULL)
//	return -1;
//    int l=CompileData(buff, buff_size, atoll(session.c_str()), atoll(terminal.c_str()), atoi(provider.c_str()), atoi(currency.c_str()), data.c_str(), data_size, type, (unsigned char)magic);
//    return l;
//}
//
//int CompileData(char *buffer, int max_size, long long id, long term, int operat, int currency, const char *data, size_t data_size, unsigned char nettype, unsigned char magic)
//{
//
//	int l=0;
//	char *ptr=buffer+2;
//	long long t_id=id;
//	long t_op=(long)operat;
//	long t_term=(long)term;
//	int t_cur=currency;
//	
//	if (max_size<=(3+sizeof(t_id)+sizeof(t_op)+sizeof(t_term)+sizeof(t_cur)+data_size))
//	    return -2;
//	
//	//TYPE
//	memcpy(&buffer[0], &nettype, 1);
//	
//	//VERSION
//	buffer[1]=0;
//	
//	
//	memcpy(ptr, &t_id, sizeof(t_id));
//	ptr+=sizeof(t_id);
//	
//	memcpy(ptr, &magic, 1);
//	ptr+=1;
//	
//	memcpy(ptr, &t_op, sizeof(t_op));
//	ptr+=sizeof(t_op);
//	
//	memcpy(ptr, &t_term, sizeof(t_term));
//	ptr+=sizeof(t_term);
//
//	memcpy(ptr, &t_cur, sizeof(t_cur));
//	ptr+=sizeof(t_cur);
//
//	memcpy(ptr, data, data_size);
//	ptr+=data_size;
//	l=ptr-buffer;
//	return l;
//
//}
