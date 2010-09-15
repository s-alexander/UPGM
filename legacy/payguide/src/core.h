#ifndef __module_core__
#define __module_core__
#include <iostream>
#include <fstream>
#include <mysql/mysql.h>
#include <mysql/mysql_com.h>
#include <mysql/errmsg.h>
#include <mysql/mysql_version.h>
#include <mysql/mysqld_error.h>
#include "log.h"
#include "pay.h"

int WIN1251TOKOI8R(const char *win1251_msg, char *buff, size_t buff_size);
char *WIN1251TOKOI8R(const char *win1251_msg);
/*
Convert a cp1251 string to koi8-r. You MUST delete [] result pointer manually. Ex:
    char *t=WIN1251TOKOI8R("Ïðåâåä");
    printf(t); //Will print "ðÒÅ×ÅÄ"
    delete [] t;
 */

int KOI8RTOWIN1251(const char *koi8r_msg, char *buff, size_t buff_size);
char *KOI8RTOWIN1251(const char *win1251_msg);
/* Same as WIN1251TOKOI8R, but converts koi8-r to cp1251 */

char *ConvertToUrlCode(const char *message);
/*
Conver a string to URL-coded string. You MUST delete [] result pointer manually. Ex:
    char *t=ConvertToUrlCode("Hello world");
    printf(t); //Will print "Hello%20world"
    delete [] t;
*/

int my_db_query(MYSQL *connection, char *query, const char *host, const char *user, const char *password, const char *db_name);
int my_db_query(MYSQL *connection, char *query, int q_size, const char *host, const char *user, const char *password, const char *db_name);
int my_simple_db_query(MYSQL *connection, char *query);
/*
Make a query to  MYSQL database. You shoud specify connection, query string with terminating null host name,
user, password and db_name. If connection is lost, my_db_query will reconnect.
ATTENTION! Using this function with WRONG sql query will *shutdown* payguide.
*/

size_t GetCurlData(void *buffer, size_t size, size_t nmemb, void *userp);
/*
Use this function for CURL request. Ex:
    SMemStruct data_buff;	//See below
    data_buff.data=NULL;
    data_buff.size=0;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetCurlData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data_buff);
    curl_easy_perform(curl);
    if (data_buff.data!=NULL)
    {
	printf("result: [%s]\n", data_buff.data);
	delete [] data_buff.data;
	data_buff.data=NULL;
    }
    else
    {
	printf("Timeout\n");
    }
*/


std::string NumberToHashCode(long long number);
/*
Returns A-coded number. I'm not sure you need this.
*/

/*
Pack number to buffer, use BCD (aka 2 bytes per number).
Returns num of used bytes.
*/
int iso8583_code(char *buff, int buff_size, long long number);

/*
Converts BCD (aka 2 bytes per number) packed data from buffer lenght buff_size.
Returns result decoded number.
*/
long long iso8583_decode(char *buff, int buff_size);


struct SMemStruct
{
	char *data;
	int size;
};

/*
Convert data from output with size l to Base64 foramat.
*/
void B64encode(const unsigned char* input,size_t l,std::string& output);


/*
Convert Base64 string to bianry data.
No more than outmax bytes will be written to out
Function returns amount of written data.
*/
int B64decode(const char *input, unsigned char *out, int outmax);

#define LOG_PRINTF_NORMAL 0
#define LOG_PRINTF_WARNING 1
#define LOG_PRINTF_CRITICAL 2
#define LOG_PRINTF_MESSAGEBOX 3

void log_printf(int priority, const char *message);
void log_printf(const char *message);

/*
Make a new pay.
You SHOUD destroy it, ysing opearator delete.
*/
SPay *CompileNewPay(long long pay_id, int engine, int provider,const char *stamp, const char *data, const char *bill_num, long long terminal_id, int currency, float lo_perc, float op_perc, float real_summ, float back_summ, float amount);

/*
Simple function that create MD5 hex-coded hash of 0-termainated text
*/
int CreateMD5HexSign(const char *text, char *buff, int buff_size);

/*
Generates terminal ID from range (start, end)
*/
void GenerateTerm(std::string *term, int start, int end);

/*
Returns a value of param, taken form $ separated string.
param_num begins with 0.
ParseX$Params("param1$param2$$param4$param5", 3) will return param4
*/
std::string ParseX$Params(const char *data, unsigned int param_num);

/*
Returns a substring, placed between to others substrings (tags).
XMLSimpleParse(<br>my name is alex</br><br>my name is vera</br>, <br>, </br>); will return "my name is alex"
*/
std::string XMLSimpleParse(const char *text, const char *tag_start, const char *tag_end);
#endif
