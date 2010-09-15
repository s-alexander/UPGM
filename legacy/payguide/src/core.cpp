#include <sys/types.h>
#include <math.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

#include "core.h"
#include "paycheck.h"



static const char c_win1251 []="éöóêåíãøùçõúôûâàïğîëäæıÿ÷ñìèòüáşÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏĞÎËÄÆİß×ÑÌÈÒÜÁŞ";
static const char c_koi8r []="ÊÃÕËÅÎÇÛİÚÈßÆÙ×ÁĞÒÏÌÄÖÜÑŞÓÍÉÔØÂÀêãõëåîçûıúèÿæù÷áğòïìäöüñşóíéôøâà";
static const char ISO8583NUMBER[10]={0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9};

static int RestoreSQLConnection(MYSQL *connection, const char *host, const char *user, const char *password, const char *db_name);

char *ConvertToUrlCode(const char *message)
{
	if (message==NULL)
		return NULL;
	int l=0;
	unsigned int i=0;
	for (i=0; i<strlen(message); i++)
	{
		if (isalnum(message[i])!=0 && (message[i]+256<192))
			l++;
		else
			l+=3;
	}
	char *answ = new char[l+2];
	
	l=0;
	
	for (i=0; i<strlen(message); i++)
	{
		if (isalnum(message[i])!=0 && (message[i]+256<192))
		{
			answ[l]=message[i];
			l++;
		}
		else
		{
			answ[l]='%';
			if (message[i]=='\n')		{answ[l+1]='0';answ[l+2]='A';}
			else if (message[i]=='\r')	{answ[l+1]='0';answ[l+2]='D';}
			else if (message[i]==' ')	{answ[l+1]='2';answ[l+2]='0';}
			else if (message[i]=='+')	{answ[l+1]='2';answ[l+2]='B';}
			else if (message[i]=='=')	{answ[l+1]='3';answ[l+2]='D';}
			else if (message[i]=='/')	{answ[l+1]='2';answ[l+2]='F';}
			else if (message[i]==',')	{answ[l+1]='2';answ[l+2]='C';}
			else if (message[i]=='%')	{answ[l+1]='2';answ[l+2]='5';}
			else if (message[i]=='$')	{answ[l+1]='2';answ[l+2]='4';}
			else if (message[i]==';')	{answ[l+1]='3';answ[l+2]='B';}
			else if (message[i]=='á')	{answ[l+1]='C';answ[l+2]='0';}
			else if (message[i]=='â')	{answ[l+1]='C';answ[l+2]='1';}
			else if (message[i]=='÷')	{answ[l+1]='C';answ[l+2]='2';}
			else if (message[i]=='ç')	{answ[l+1]='C';answ[l+2]='3';}
			else if (message[i]=='ä')	{answ[l+1]='C';answ[l+2]='4';}
			else if (message[i]=='å')	{answ[l+1]='C';answ[l+2]='5';}
			else if (message[i]=='³')	{answ[l+1]='A';answ[l+2]='8';}
			else if (message[i]=='ö')	{answ[l+1]='C';answ[l+2]='6';}
			else if (message[i]=='ú')	{answ[l+1]='C';answ[l+2]='7';}
			else if (message[i]=='é')	{answ[l+1]='C';answ[l+2]='8';}
			else if (message[i]=='ê')	{answ[l+1]='C';answ[l+2]='9';}
			else if (message[i]=='ë')	{answ[l+1]='C';answ[l+2]='A';}
			else if (message[i]=='ì')	{answ[l+1]='C';answ[l+2]='B';}
			else if (message[i]=='í')	{answ[l+1]='C';answ[l+2]='C';}
			else if (message[i]=='î')	{answ[l+1]='C';answ[l+2]='D';}
			else if (message[i]=='ï')	{answ[l+1]='C';answ[l+2]='E';}
			else if (message[i]=='ğ')	{answ[l+1]='C';answ[l+2]='F';}
			else if (message[i]=='ò')	{answ[l+1]='D';answ[l+2]='0';}
			else if (message[i]=='ó')	{answ[l+1]='D';answ[l+2]='1';}
			else if (message[i]=='ô')	{answ[l+1]='D';answ[l+2]='2';}
			else if (message[i]=='õ')	{answ[l+1]='D';answ[l+2]='3';}
			else if (message[i]=='æ')	{answ[l+1]='D';answ[l+2]='4';}
			else if (message[i]=='è')	{answ[l+1]='D';answ[l+2]='5';}
			else if (message[i]=='ã')	{answ[l+1]='D';answ[l+2]='6';}
			else if (message[i]=='ş')	{answ[l+1]='D';answ[l+2]='7';}
			else if (message[i]=='û')	{answ[l+1]='D';answ[l+2]='8';}
			else if (message[i]=='ı')	{answ[l+1]='D';answ[l+2]='9';}
			else if (message[i]=='ÿ')	{answ[l+1]='D';answ[l+2]='A';}
			else if (message[i]=='ù')	{answ[l+1]='D';answ[l+2]='B';}
			else if (message[i]=='ø')	{answ[l+1]='D';answ[l+2]='C';}
			else if (message[i]=='ü')	{answ[l+1]='D';answ[l+2]='D';}
			else if (message[i]=='à')	{answ[l+1]='D';answ[l+2]='E';}
			else if (message[i]=='ñ')	{answ[l+1]='D';answ[l+2]='F';}
			else if (message[i]=='Á')	{answ[l+1]='E';answ[l+2]='0';}
			else if (message[i]=='Â')	{answ[l+1]='E';answ[l+2]='1';}
			else if (message[i]=='×')	{answ[l+1]='E';answ[l+2]='2';}
			else if (message[i]=='Ç')	{answ[l+1]='E';answ[l+2]='3';}
			else if (message[i]=='Ä')	{answ[l+1]='E';answ[l+2]='4';}
			else if (message[i]=='Å')	{answ[l+1]='E';answ[l+2]='5';}
			else if (message[i]=='£')	{answ[l+1]='B';answ[l+2]='8';}
			else if (message[i]=='Ö')	{answ[l+1]='E';answ[l+2]='6';}
			else if (message[i]=='Ú')	{answ[l+1]='E';answ[l+2]='7';}
			else if (message[i]=='É')	{answ[l+1]='E';answ[l+2]='8';}
			else if (message[i]=='Ê')	{answ[l+1]='E';answ[l+2]='9';}
			else if (message[i]=='Ë')	{answ[l+1]='E';answ[l+2]='A';}
			else if (message[i]=='Ì')	{answ[l+1]='E';answ[l+2]='B';}
			else if (message[i]=='Í')	{answ[l+1]='E';answ[l+2]='C';}
			else if (message[i]=='Î')	{answ[l+1]='E';answ[l+2]='D';}
			else if (message[i]=='Ï')	{answ[l+1]='E';answ[l+2]='E';}
			else if (message[i]=='Ğ')	{answ[l+1]='E';answ[l+2]='F';}
			else if (message[i]=='Ò')	{answ[l+1]='F';answ[l+2]='0';}
			else if (message[i]=='Ó')	{answ[l+1]='F';answ[l+2]='1';}
			else if (message[i]=='Ô')	{answ[l+1]='F';answ[l+2]='2';}
			else if (message[i]=='Õ')	{answ[l+1]='F';answ[l+2]='3';}
			else if (message[i]=='Æ')	{answ[l+1]='F';answ[l+2]='4';}
			else if (message[i]=='È')	{answ[l+1]='F';answ[l+2]='5';}
			else if (message[i]=='Ã')	{answ[l+1]='F';answ[l+2]='6';}
			else if (message[i]=='Ş')	{answ[l+1]='F';answ[l+2]='7';}
			else if (message[i]=='Û')	{answ[l+1]='F';answ[l+2]='8';}
			else if (message[i]=='İ')	{answ[l+1]='F';answ[l+2]='9';}
			else if (message[i]=='ß')	{answ[l+1]='F';answ[l+2]='A';}
			else if (message[i]=='Ù')	{answ[l+1]='F';answ[l+2]='B';}
			else if (message[i]=='Ø')	{answ[l+1]='F';answ[l+2]='C';}
			else if (message[i]=='Ü')	{answ[l+1]='F';answ[l+2]='D';}
			else if (message[i]=='À')	{answ[l+1]='F';answ[l+2]='E';}
			else if (message[i]=='Ñ')	{answ[l+1]='F';answ[l+2]='F';}
			else
			{
				answ[l]=message[i];
				l-=2;
			}
			l+=3;
		}
		
	}
	answ[l]='\0';
	return answ;
}

size_t GetCurlData(void *buffer, size_t size, size_t nmemb, void *userp)
{
	if (userp==NULL) return 0;
	SMemStruct *buff=(SMemStruct *)userp;

	if (buff->data==NULL)
	{
		buff->data=new char[nmemb*size+1];
		strncpy(buff->data, (char *)buffer, nmemb);
		buff->data[nmemb]='\0';
		buff->size=nmemb+1;
	}
	else
	{
		char *t=new char[buff->size+(nmemb*size)+1];
		if (t==NULL) return 0;
		strncpy(t,buff->data, buff->size);
		delete [] buff->data;
		buff->data=t;
		strncat(buff->data, (char *)buffer, nmemb);
		buff->data[buff->size+nmemb]='\0';
		buff->size+=nmemb;
	
	}
	return nmemb;
}

std::string NumberToHashCode(long long number)
{
const int MAX_HASH_NUMBER=52;
const char HASH_CODES[]="QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";

	std::string answ="";
	char buff[128];
	snprintf(buff, 127, "%lli",number);

	for (unsigned int a=0; a<strlen(buff); a++)
	{
		if (a<strlen(buff)-1)
		{
			char t1[2];t1[0]=buff[a];t1[1]='\0';
			char t2[2];t2[0]=buff[a+1];t2[1]='\0';

			int num=atoi(t1)*10+atoi(t2);

			if (num<MAX_HASH_NUMBER)
			{
				answ+=HASH_CODES[num];
				a++;
			}
			else
			{
				answ+=buff[a];		
			}
		}
		else
		{
			answ+=buff[a];
		}
	}
	return answ;
}

int WIN1251TOKOI8R(const char *win1251_msg, char *buff, size_t buff_size)
{
	if (win1251_msg==NULL)
		return -1;
	unsigned int l=strlen(win1251_msg)+1;
	if (l>buff_size)
		return -1;
	char *koi8r_msg=buff;
	unsigned int l_alphabet=strlen(c_win1251);
	bool found=false;
	for (unsigned int a=0; a<l; a++)
	{
		for (unsigned int b=0; b<l_alphabet; b++)
		{
			found=false;
			if (win1251_msg[a]==c_win1251[b])
			{
				koi8r_msg[a]=c_koi8r[b];
				b=l_alphabet;
				found=true;
				
			}
		}
		if (found==false)
			koi8r_msg[a]=win1251_msg[a];
	}
	return 0;

}


char *WIN1251TOKOI8R(const char *win1251_msg)
{
	if (win1251_msg==NULL)
		return NULL;
	
	int l=strlen(win1251_msg);
	char *koi8r_msg = new char [l+1];
	if (koi8r_msg!=NULL)
		WIN1251TOKOI8R(win1251_msg, koi8r_msg, l+1);
	return koi8r_msg;
}

int KOI8RTOWIN1251(const char *koi8r_msg, char *buff, size_t buff_size)
{
	if (koi8r_msg==NULL)
		return -1;
	unsigned int l=strlen(koi8r_msg)+1;
	if (l>buff_size)
		return -1;
	char *win1251_msg=buff;
	unsigned int l_alphabet=strlen(c_koi8r);
	bool found=false;
	for (unsigned int a=0; a<l; a++)
	{
		for (unsigned int b=0; b<l_alphabet; b++)
		{
			found=false;
			if (koi8r_msg[a]==c_koi8r[b])
			{
				win1251_msg[a]=c_win1251[b];
				b=l_alphabet;
				found=true;
				
			}
		}
		if (found==false)
			win1251_msg[a]=koi8r_msg[a];
	}
	return 0;

}

char *KOI8RTOWIN1251(const char *koi8r_msg)
{
	if (koi8r_msg==NULL)
		return NULL;
	
	int l=strlen(koi8r_msg);
	char *win1251_msg = new char [l+1];
	if (win1251_msg!=NULL)
		KOI8RTOWIN1251(koi8r_msg, win1251_msg, l+1);
	return win1251_msg;
}

int my_db_query(MYSQL *connection, char *query, const char *host, const char *user, const char *password, const char *db_name)
{
	return my_db_query(connection, query, strlen(query), host, user, password, db_name);
}

int my_db_query(MYSQL *connection, char *query, int q_size, const char *host, const char *user, const char *password, const char *db_name)
{
	if (connection==NULL || query==NULL) return -1;

	int err=0;
	int sql_err=0;

	err=mysql_real_query(connection, query, q_size);
	
	/* Query not complited */
	if (err!=0)
	{
		sql_err=mysql_errno(connection);
		
		/* Wrong MYSQL query. Shutdown. */
		if (sql_err==ER_PARSE_ERROR ||  sql_err==ER_BAD_FIELD_ERROR || sql_err==ER_EMPTY_QUERY || ER_NO_SUCH_TABLE==sql_err)
		{
			char logmsg[1024];
			snprintf(logmsg, 1024, "Got FATAL error processing [%s], code %i (bad query?). Payguide shutdown.", query, sql_err);
			LogWrite(LOGMSG_CRITICAL, logmsg);
			exit(-1);
		}
		
		if (sql_err==ER_DUP_ENTRY)
		{
			return sql_err;	
		}
		
//		char logmsg[1024];
		
		while (err!=0)
		{
			
			int restore_result=-1;
			int restore_count=1;
			while (restore_result!=0)
			{
//				snprintf(logmsg, 1024, "Got error processing [%s], code %d. Restoring MYSQL connection, attemp %i...", query, sql_err, restore_count);
//				LogWrite(LOGMSG_ERROR, logmsg);
				restore_result=RestoreSQLConnection(connection, host, user, password, db_name);
				if (restore_count<3200)
					restore_count++;
//				else
//					LogWrite(LOGMSG_CRITICAL, "WARNING! failed to restore Mysql connection after 3200 attemps.");
				if (CR_SERVER_GONE_ERROR==sql_err || CR_SERVER_LOST==sql_err || CR_SERVER_HANDSHAKE_ERR==sql_err)
					sleep(1);
				else
				{
//					LogWrite(LOGMSG_ERROR, "(Not CR_SERVER_GONE_ERROR, CR_SERVER_LOST, CR_SERVER_HANDSHAKE_ERR error, sleeping 1 min)");
					sleep(60);
				}
			}
//			LogWrite(LOGMSG_SYSTEM, "Connection restored, calling my_db_query again.");
			err=mysql_real_query(connection, "set names koi8r", strlen("set names koi8r"));
			if (err==0)
				err=mysql_real_query(connection, query, q_size);
			sql_err=mysql_errno(connection);
		}
	}
	return 0;
}

int RestoreSQLConnection(MYSQL *connection, const char *host, const char *user, const char *password, const char *db_name)
{
	MYSQL *t=NULL;
	if (connection==NULL)
		return -1;		
	mysql_close(connection);
	mysql_init(connection);
	t=mysql_real_connect(connection, host, user, password, db_name, 0, NULL, 0);
	if (t!=NULL)
		return 0;
	else
		return -1;
}



void B64encode(const unsigned char* input,size_t l,std::string& output)
{
	const char *bstr ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        size_t i = 0;
	size_t o = 0;
         
        output = "";
        while (i < l)
        {
		size_t remain = l - i;
                switch (remain)
                {
                case 1:
            		output += bstr[ ((input[i] >> 2) & 0x3f) ];
                        output += bstr[ ((input[i] << 4) & 0x30) ];
                        output += "==";
                        break;
                case 2:
                        output += bstr[ ((input[i] >> 2) & 0x3f) ];
                        output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
                        output += bstr[ ((input[i + 1] << 2) & 0x3c) ];
                        output += "=";
                        break;
                default:
                        output += bstr[ ((input[i] >> 2) & 0x3f) ];
                        output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
                        output += bstr[ ((input[i + 1] << 2) & 0x3c) + ((input[i + 2] >> 6) & 0x03) ];
                        output += bstr[ (input[i + 2] & 0x3f) ];
                }
                o += 4;
                i += 3;
	}
}

int B64decode(const char *input, unsigned char *out, int outmax)
{
	if (input==NULL || out==NULL)
		return -1;
	int result=0;
	const char *bstr ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const char *ptr=input;
	unsigned char *optr=out;
	for (unsigned int i=0;i<strlen(input); i+=4)
	{
		unsigned char outb[3];
		int left=strlen(input)-i;
		if (left>3)
			left=4;
		memset(outb, 0, 3);
		for (unsigned int k=0; k<left; k++)
		{
			for (unsigned int j=0; j<strlen(bstr); j++)
			{
				if (ptr[k]==bstr[j])
				{
					
					if (k==0)
					{
						outb[0]=j;
						outb[0]=outb[0]<<2;
					}
					
					if (k==1)
					{
						outb[1]=j;
						outb[1]=outb[1]<<2;
						unsigned char t=outb[1];
						t=t>>6;
						outb[0]=outb[0] | t;
						outb[1]=outb[1]<<2;
						char tmp[2];
						tmp[0]=outb[0]; tmp[1]=0;
					}
					
					if (k==2)
					{
						outb[2]=j;
						outb[2]=outb[2] << 2;
						unsigned char t=outb[2]>>4;
						outb[1]=outb[1] | t;
						outb[2]=outb[2]<<4;
					}
					
					if (k==3)
					{
						unsigned char t=j;
						t=t>>2;
						outb[2]=outb[2] | j;
					}
				}
			}
		}
		if (left>0)
		{
			if (result<outmax)
			{
				memcpy(optr, outb, 3);
				result+=3;
				optr+=3;
			}
			else
				return -1;
		}
		ptr+=left;
	}
	return result;
	
}

void B64encodeMS(const unsigned char* input,size_t l,std::string& output)
{
	const char *bstr ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        size_t i = 0;
	size_t o = 0;
         
        output = "";
        while (i < l)
        {
		size_t remain = l - i;
                switch (remain)
                {
                case 1:
            		output += bstr[ ((input[i] >> 2) & 0x3f) ];
                        output += bstr[ ((input[i] << 4) & 0x30) ];
                        output += "==";
                        break;
                case 2:
                        output += bstr[ ((input[i] >> 2) & 0x3f) ];
                        output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
                        output += bstr[ ((input[i + 1] << 2) & 0x3c) ];
                        output += "=";
                        break;
                default:
                        output += bstr[ ((input[i] >> 2) & 0x3f) ];
                        output += bstr[ ((input[i] << 4) & 0x30) + ((input[i + 1] >> 4) & 0x0f) ];
                        output += bstr[ ((input[i + 1] << 2) & 0x3c) + ((input[i + 2] >> 6) & 0x03) ];
                        output += bstr[ (input[i + 2] & 0x3f) ];
                }
                o += 4;
                i += 3;
	}
}

void log_printf(int priority, const char *message)
{
	printf("%s\n", message);
//	SendLogMessages(priority, message);
}

void log_printf(const char *message)
{
	log_printf(LOG_PRINTF_NORMAL, message);
}

SPay *CompileNewPay(long long pay_id, int engine, int provider,const char *stamp, const char *data, const char *bill_num, long long terminal_id, int currency, float lo_perc, float op_perc, float real_summ, float back_summ, float amount)
{

	SPay *pay=new SPay();
	if (pay==NULL)
		return NULL;
	
	pay->id=pay_id;
	pay->pay_engine=engine;
	pay->provider_id=provider;
	pay->currency=currency;
	pay->paysys_codename=NULL;
	pay->test=NO_TEST;
	if (stamp!=NULL) strncpy(pay->stamp, stamp,SIZE_STAMP); else strncpy(pay->stamp, "",SIZE_STAMP);
	if (data!=NULL) strncpy(pay->data, data,SIZE_DATA); else strncpy(pay->data, "",SIZE_DATA);
	if (bill_num!=NULL) strncpy(pay->bill_num, bill_num,SIZE_BILL_NUM); else strncpy(pay->bill_num, "",SIZE_BILL_NUM);
	pay->terminal_id=terminal_id;
	sem_init(&pay->pay_canceled_sem, 0,1);
	pay->pay_canceled=0;
	pay->lo_perc=lo_perc;
	pay->op_perc=op_perc;
	pay->summ=real_summ;
	pay->amount=amount;
	pay->back_summ=back_summ;
	pay->exists_int_checkreqs=0;
	pay->nettype=0x01;
	pay->netversion=0x00;

	return pay;
}

int iso8583_code(char *buff, int buff_size, long long number)
{
	char tmp[256];
	snprintf(tmp,256, "%lli", number);
	char n[2];n[1]=0;
	int count=0;
	if (buff_size<=0)
		return 0;
	char c;
	char *p=tmp;
	int step=1;
	int a=0;
	if (strlen(tmp)%2==1)
	{
		c=0x0;
		step=2;
	}
	while (p[0]!=0 && count<buff_size)
	{
		if (step==1)
		{
			n[0]=p[0];
			a=atoi(n);
			c=ISO8583NUMBER[a];
			step=2;
		}
		else
		{
			
			c=c<<4 ;
			n[0]=p[0];
			a=atoi(n);
			c=c|ISO8583NUMBER[a];
			buff[count]=c;
			count++;
			step=1;
		}
		p++;
		
	}
	return count;
}

long long iso8583_decode(char *buff, int buff_size)
{
	
	char tmp[256];
	long long result=0;
	tmp[0]=0;
	int count=0;
	if (buff_size<=0)
		return 0;
	char c;
	char n[2]; n[1]=0;
	char t[2]; t[1]=0;
	char *p=buff;
	int a=0;
	int step=buff_size*2-1;
	while (count<buff_size)
	{
		c=p[0];
		a=0x0|((c>>4)&0xF);
		if (step!=0)
			result=result+a*(long long)(pow(10,step));
		else
		{
			result=result+a;
			if (a==0)
				step--;
		}
		
		a=c&0xF;
		step--;
		result=result+a*(long long)(pow(10,step));
		p++;
		count++;
		step--;

	}
	return result;
}

int my_simple_db_query(MYSQL *connection, char *query)
{
	if (connection==NULL || query==NULL) return -1;

	int err=mysql_real_query(connection, query, strlen(query));
	if (err!=0)
	{
		int sql_err=mysql_errno(connection);
		if (CR_SERVER_GONE_ERROR==sql_err || CR_SERVER_LOST==sql_err)
		{
			mysql_ping(connection);
			return mysql_real_query(connection, query, strlen(query));
		}
		else
		{
			return err;
		}
	}
	return err;
}

int CreateMD5HexSign(const char *text, char *buff, int buff_size)
{
	memset(buff, 0, buff_size);
	int i;
	if (buff_size<MD5_DIGEST_LENGTH*8+1)
		return -1;
	unsigned char md5digest[MD5_DIGEST_LENGTH];

	MD5((unsigned char *)text,strlen(text), md5digest);

	for (i=0;i<MD5_DIGEST_LENGTH;i++) 
	{
		char tmps[10];
		snprintf(tmps,9, "%02x",  md5digest[i]);
		strncat(buff, tmps, buff_size);
	}
	return 0;
}

void GenerateTerm(std::string *term, int start, int end)
{
	if (term==NULL)
		return;
	char tmp[512];
	if (end-start>1)
	{
		int value=start+rand()%abs(end-start-1)+1;
		memset(tmp, 0,512);
		snprintf(tmp, 511, "%i", value);
		*term=tmp;
	}
	else
	{
		snprintf(tmp, 511, "%i", start);
		*term=tmp;
	}
	return;
}

std::string ParseX$Params(const char *data, unsigned int param_num)
{
	std::string result;
	if (data==NULL)
		return result;
		
	const char *ptr=strstr(data, "$");
	const char *ptr_prev=data;
	unsigned int c=0;
	while (c<param_num && ptr!=NULL)
	{
		ptr_prev=ptr;
		ptr=strstr(ptr+1, "$");
		c++;
	}
	
	if (c==param_num && ptr==NULL && ptr_prev!=NULL)
	{
		if (c>0)
			ptr_prev++;
		result.assign(ptr_prev);
	}
	else if (c==param_num && ptr_prev!=NULL && ptr!=NULL)
	{
		if (c>0)
			ptr_prev++;
		result.assign(ptr_prev, ptr-ptr_prev);
	}
	return result;
}

std::string XMLSimpleParse(const char *text, const char *tag_start, const char *tag_end)
{
	
	std::string result;
	if (text==NULL || tag_start==NULL || tag_end==NULL)
		return result;
	const char *ptr1=strstr(text, tag_start);
	if (ptr1!=NULL)
	{
		ptr1+=strlen(tag_start);
		const char *ptr2=strstr(ptr1, tag_end);
		if (ptr2!=NULL)
		{
			result.assign(ptr1, ptr2-ptr1);
		}
	}
	return result;

}
