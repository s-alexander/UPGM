#include "cpaycheckcore.h"
#include "cpayguiderequest.h"
#include "../namespace.h"
#include "../pay.h"
#include "../core.h"

static int GetPayguideStatReport(std::string &buff);
static std::string http_header="HTTP/1.1 200 OK\nServer: paycheck\nAccept-Ranges: bytes\nConnection: close\n\n";
static std::string xml_header="<?xml version=\"1.0\" encoding=\"KOI8-R\"?>\n";
const char *localhost_addr="127.0.0.1";

paycheck::CPaycheckCore::CPaycheckCore()
{
    trusted_net=false;
}

paycheck::CPaycheckCore::~CPaycheckCore()
{
    
}

int paycheck::CPaycheckCore::LoadConfig(const char *config_filename)
{
    if (config_filename==NULL)
	    return -1;
//    printf("Loading main config from [%s]\n", config_filename);
    main_cfg.ReadFile(config_filename);
    
    
    const char *op_allow_fn=main_cfg.GetValue("operators_allow", "xml_paycheck");
    if (op_allow_fn!=NULL)
    {
	op_allow.ReadFile(op_allow_fn);
    }
    else
    {
	//printf("operators_allow doesnt set - deny all\n");
    }
    
    const char *tnet=main_cfg.GetValue("trusted", "xml_paycheck");
    if (tnet!=NULL)
    {
	    if (strcmp(tnet, "yes")==0 || strcmp(tnet, "1")==0 || strcmp(tnet, "y")==0)
	    {
		trusted_net=true;
	    }
    }

    return 0;
}

const char *paycheck::CPaycheckCore::GetNetworkDevice()
{
	if (main_cfg.GetValue("network_device", "xml_paycheck")!=NULL)
		return main_cfg.GetValue("network_device", "xml_paycheck");
	return localhost_addr;
}

long long int paycheck::CPaycheckCore::GetNetworkPort()
{
	if (main_cfg.GetValue("port", "xml_paycheck")!=NULL)
		return atoll(main_cfg.GetValue("port", "xml_paycheck"));
	return 25001;
}


int paycheck::CPaycheckCore::SimpleAnswer(std::string &content, paycheck::CSocket &socket)
{
    char c_size[128];
    snprintf(c_size, 128, "%i", content.length());
    std::string answer="HTTP/1.1 200 OK\nServer: paycheck\nAccept-Ranges: bytes\nContent-Length: ";
    answer+=c_size;
    answer+="\nContent-Type: text/plain\n\n";
    answer+=content;

    socket.Send(answer.c_str());
    //printf("OUT [%s]\n", answer.c_str());
    socket.ForceDown();
    return 0;

}

int paycheck::CPaycheckCore::GenericError(int error_code, paycheck::CSocket &socket)
{
    char c_size[128];
    std::string content;
    if (error_code==G_ERROR_START_PAGE)
    {
    content=xml_header+"<SystemError><error>1</error><comment>Wrong request</comment></SystemError>";
    }
    else if (error_code==G_ERROR_QUEUE_FULL)
    {
	content=xml_header+"<SystemError><error>2</error><comment>XMLPaycheck queue is full. Please, try again later.</comment></SystemError>";
    }
    else if (error_code==G_ERROR_NO_ACTION)
    {
	content=xml_header+"<SystemError><error>3</error><comment>Action not specified.</comment></SystemError>";
    }
    else if (error_code==G_ERROR_ACTION_NOT_FOUND)
    {
	content=xml_header+"<SystemError><error>4</error><comment>No such action</comment></SystemError>";
    }
    else if (error_code==G_ERROR_NOT_ALL_PARAMS)
    {
    	content=xml_header+"<SystemError><error>5</error><comment>Not all params specified for this action</comment></SystemError>";
    }
    else if (error_code==G_ERROR_ACTION_WRONG_OP)
    {
    	content=xml_header+"<SystemError><error>6</error><comment>This operator not supported</comment></SystemError>";
    }
    else if (error_code==G_ERROR_WRONG_METHOD)
    {
    	content=xml_header+"<SystemError><error>7</error><comment>Method not allowed or message is too big</comment></SystemError>";
    }
    else
    {
    	content=xml_header+"<SystemError><error>1000</error><comment>Payguide internal error.</comment></SystemError>";
    }
    snprintf(c_size, 128, "%i", content.length());
    std::string answer="HTTP/1.1 200 OK\nServer: paycheck\nAccept-Ranges: bytes\nContent-Length: ";
    answer+=c_size;
    answer+="\nContent-Type: text/html\n\n";
    answer+=content;

    socket.Send(answer.c_str());
    //printf("OUT [%s]\n", answer.c_str());
    socket.ForceDown();
    return 0;
}

int paycheck::CPaycheckCore::Xml2PayguideFormat(std::string &xml_txt, paycheck::CSocket &sock)
{
    int result=-1;
    //printf("parsing [%s]...\n", xml_txt.c_str());
    std::string op=XMLSimpleParse(xml_txt.c_str(), "operator=", "\n");
    std::string data=XMLSimpleParse(xml_txt.c_str(), "data=", "\n");
    std::string session=XMLSimpleParse(xml_txt.c_str(), "session=", "\n");
    std::string cur=XMLSimpleParse(xml_txt.c_str(), "currency=", "\n");
    std::string action=XMLSimpleParse(xml_txt.c_str(), "action=", "\n");
    std::string terminal_id=XMLSimpleParse(xml_txt.c_str(), "terminal=", "\n");
    
    if (action!="")
    {
	if (action=="check")
	{
	    if (op!="" && data!="" && session!="" && cur!="" && terminal_id!="")
	    {
				if (op_allow.GetValue(op.c_str(), "ALL")!=NULL && strcmp(op_allow.GetValue(op.c_str(), "ALL"),"allow")==0)
				{
				    SPayResult pres;
				    char stamp[64];
				    timeval time_now;
				    gettimeofday(&time_now, NULL);
				    tm *ptm=localtime (&time_now.tv_sec);
				    strftime(stamp, 64, "%Y-%m-%d %H:%M:%S", ptm);
				    SPay *new_pay=CompileNewPay(atoll(session.c_str()), 10, atoi(op.c_str()) , stamp, data.c_str(), 0, atoll(terminal_id.c_str()), atoi(cur.c_str()), 0, 0, -1, -1, -1);
				    if (new_pay!=NULL)
				    {
					    new_pay->xml_output=true;
					    new_pay->test=FULL_TEST;
					    new_pay->xml_sock=&sock;

			    		    bool already_checked=false;

					    char pay_query[512];
					    snprintf(pay_query,512,"select error, result from check_reqs where bill_num='%lli' and tid=%lli", new_pay->id, new_pay->terminal_id);
					    sem_wait(&payguide::db_lock_checkreqs);
					    my_db_query(&payguide::db_checkreqs_connection, pay_query, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str());
					    MYSQL_RES *quest_res=NULL;
					    quest_res=mysql_store_result(&payguide::db_checkreqs_connection);
					    if (quest_res!=NULL)
					    {
					    	MYSQL_ROW row;
					    	row = mysql_fetch_row(quest_res);
					    	unsigned long *lengths=mysql_fetch_lengths(quest_res);
					    	if (row!=NULL)
					    	{
					    		already_checked=true;
							pres.code=RESULT_UNKNOWN;
					    		if (row[0]!=NULL && row[1]!=NULL)
					    		{
					    			pres.code=atoi(row[0]);
					    			if (lengths[1]<SIZE_REPONSE_MSG)
					    			{
					    				memcpy(pres.msg, row[1], lengths[1]);
					    				if (pres.code==RESULT_SUCESS_BIN)
					    				{
					    					pres.bdata_size=lengths[1];
					    				}
					    			}
								new_pay->exists_int_checkreqs=true;
					    		}
				    
				    		}
				    		mysql_free_result(quest_res);
				    	}
				    	sem_post(&payguide::db_lock_checkreqs);

					    
					    if (already_checked)
					    {
						    strncpy(pres.sender_name, "check_reqs", SIZE_REPONSE_SENDER);
						    XMLSetState(new_pay, &pres);
						    sem_post(&payguide::db_lock_checkreqs);
						    delete new_pay;

					    }
					    else
					    {

						    queue_lock.Lock();
						    if (pays.size()<1024)
						    {
							    result=0;
							    pays.push(new_pay);
							    char pay_query[512];
							    snprintf(pay_query,512,"insert into check_reqs(bill_num, tid, stamp, error) values('%lli', %lli, now(), %i)", new_pay->id, new_pay->terminal_id, RESULT_UNKNOWN);
							    my_db_query(&payguide::db_checkreqs_connection, pay_query, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str());
						    }
						    else 
						    {
							    GenericError(G_ERROR_QUEUE_FULL, sock);
							    delete new_pay;
						    }
						    sem_post(&payguide::db_lock_checkreqs);
						    queue_lock.UnLock();
					    }
				    }
					
				}
				else
				{
				    //Operator denied
				    GenericError(G_ERROR_ACTION_WRONG_OP,sock);
				}
	    }
	    else
		GenericError(G_ERROR_NOT_ALL_PARAMS, sock);
	}
	else if (action=="status")
	{
		//Show status about payguide
		//printf("STATUS\n");
		std::string answer;
		GetPayguideStatReport(answer);
		SimpleAnswer(answer, sock);
	}
	else
		GenericError(G_ERROR_ACTION_NOT_FOUND,sock);
    }
    else
    {
	GenericError(G_ERROR_NO_ACTION, sock);
    }
    
    return result;
}


static int GetPayguideStatReport(std::string &buff)
{
	int acc_new_pays=0;
	sem_wait(&payguide::db_lock_read);
	if (payguide::db_accept_new_pays==1) acc_new_pays=1;
	sem_post(&payguide::db_lock_read);


	sem_wait(&payguide::free_workers_lock);

	if (payguide::workers_list==NULL) /* Rebooting */
	{
		sem_post(&payguide::free_workers_lock);
		return 0;
	}
	char tmp[256];
	
	buff=xml_header+"<StatusRequest>";
	
	if (acc_new_pays)
	{
		buff+="<accept_pays>\n<mysql>yes</mysql>\n<bin>yes</bin>\n<xml>yes</xml>\n</accept_pays>";
	}
	else
	{
		buff+="<accept_pays>\n<mysql>no</mysql>\n<bin>no</bin>\n<xml>no</xml>\n</accept_pays>\n";
	}
	/*buff+=tmp;
	snprintf(tmp, 256,"threads total: %i<br>", payguide::workers_list->GetLen());buff+=tmp;
	snprintf(tmp, 256,"sleeping threads: %i<br>", payguide::workers_list->GetLen()-payguide::working_workers);buff+=tmp;
	snprintf(tmp, 256,"working threads: %i<br>", payguide::working_workers);buff+=tmp;*/
	
	

	std::string modules;
	payguide::modules_list->ResetCursor();
	unsigned int m_count=0;
	for (unsigned int i=0; i<payguide::modules_list->GetLen(); i++)
	{
		CPaySys *w=payguide::modules_list->GetNext();
		if (w!=NULL)
		{
			if (w->IsUnloaded()==0 && w->IsBroken()==0 && w->GetShortName()!=NULL)
			{
				
			    	snprintf(tmp, 256,"<module id=\"%i\">%s</module>\n",m_count,w->GetShortName());
				modules+=tmp;
				m_count++;
			}
		}
	}
	snprintf(tmp, 256,"<modules loaded=\"%i\">\n",m_count);
	buff+=tmp;
	buff+=modules;
	buff+="</modules>\n";



/*	time_t time_now=time(NULL);
	time_now=time_now-payguide::time_start;
	long long t_n=0;
	memcpy(&t_n, &time_now, sizeof(time_now));
	memcpy(ptr, &t_n, sizeof(t_n)); ptr+=sizeof(t_n);*/
	
	payguide::workers_list->ResetCursor();
	snprintf(tmp, 256,"<threads total=\"%i\" working=\"%i\">\n",payguide::workers_list->GetLen(), payguide::working_workers);
	buff+=tmp;
	
	for (unsigned int i=0; i<payguide::workers_list->GetLen(); i++)
	{
		CWorker *w=payguide::workers_list->GetNext();
		if (w!=NULL)
		{
			int id=w->GetId();
			int lt=w->GetLifeTime();
			const char *test_pay="regular";
			if (w->Busy()==1)
			{
				w->GetPay()->lock.LockRead();
				if (w->GetPay()->test!=NO_TEST)
					test_pay="check";
				long long pid= w->GetPay()->id;
				w->GetPay()->lock.UnLockRead();
				
				char *ps_name=w->GetPaySys()->GetShortName();
				snprintf(tmp, 256,"<thread id=\"%i\"><worker id=\"%i\" state=\"working\" pay_type=\"%s\" time=\"%i\" paysys=\"%s\">%lli</worker></thread>\n",i, id, test_pay, lt, ps_name, pid);buff+=tmp;
			
			}
			else
			{
				snprintf(tmp, 256,"<thread id=\"%i\"><worker id=\"%i\" state=\"sleeping\" time=\"%i\" paysys=\"(null)\">-1</worker></thread>\n",i, id, lt);buff+=tmp;
			}
		
		}
	}
	buff+="</threads>\n";
	
	time_t time_now=time(NULL);
	time_now=time_now-payguide::time_start;
	long long t_n=0;
	memcpy(&t_n, &time_now, sizeof(time_now));
	
	snprintf(tmp, 256,"<stat><pays_total>%lli</pays_total><uptime>%lli</uptime></stat>",payguide::pays_total, t_n);
	buff+=tmp;
	buff+="</StatusRequest>";
	
	sem_post(&payguide::free_workers_lock);
	return 0;
}

SPay *paycheck::CPaycheckCore::XMLGetNextPay()
{
	    SPay *result=NULL;
	    sem_wait(&payguide::db_lock_read);
	    if (payguide::db_accept_new_pays==0)
	    {
		sem_post(&payguide::db_lock_read);
		return NULL;
	    }
	    sem_post(&payguide::db_lock_read);

	    queue_lock.Lock();
	    if (pays.size()>0)
	    {
		    result=pays.front();
		    pays.pop();
	    }
	    queue_lock.UnLock();
	    return result;

}

int paycheck::CPaycheckCore::XMLSetState(SPay *pay, SPayResult *payres)
{
	const char *soname=NULL;

	if (pay->paysys_codename!=NULL)
		soname=pay->paysys_codename;
	else
		soname="(null)";
	if (pay->exists_int_checkreqs)
		soname="check_reqs";
	int msglen=0;
	if (payres->code==RESULT_SUCESS_BIN)
		msglen=payres->bdata_size;
	else
		msglen=strlen(payres->msg);
		
	char pay_query[4096];
	char req[1024];
	snprintf(req,1024,",error=%i where tid='%lli' and bill_num='%lli'",payres->code, pay->terminal_id, pay->id);
	sem_wait(&payguide::db_lock_checkreqs);
	char *end = pay_query;
	strncpy(pay_query,"update check_reqs set result=",4096);
	end+=strlen(pay_query);
	*end++='\'';
	end += mysql_real_escape_string(&payguide::db_checkreqs_connection, end, payres->msg, msglen);
	*end++='\'';
	strncat(end, req, 3000);
	end+=strlen(req);
	my_db_query(&payguide::db_checkreqs_connection, pay_query, end-pay_query, payguide::host_saved.c_str(), payguide::user_saved.c_str(), payguide::passwor_saved.c_str(), payguide::db_name_saved.c_str());
	sem_post(&payguide::db_lock_checkreqs);	


	if (pay->xml_sock!=NULL)
	{
		paycheck::CSocket *sock=(paycheck::CSocket *)pay->xml_sock;
		char buff[4097];memset(buff, 0, 4097);
		std::string answer(xml_header);
		answer+="<CheckRequest>";
		if (payres->code!=RESULT_SUCESS_BIN)
		{
			snprintf(buff, 4096, "<operator>%i</operator><paysys>%s</paysys><sender>%s</sender><code>%i</code><description>%s</description>", pay->provider_id, soname, payres->sender_name, payres->code, payres->msg);
		}
		else
		{
			std::string b64;
			B64encode((unsigned char *)payres->msg, payres->bdata_size, b64);
			snprintf(buff, 4096, "<operator>%i</operator><paysys>%s</paysys><sender>%s</sender><code>%i</code><description></description><nettype>%i</nettype><netversion>%i</netversion><data>%s</data>", pay->provider_id, soname, payres->sender_name, payres->code, (unsigned int)pay->nettype, (unsigned int)pay->netversion, b64.c_str());
		}
		answer+=buff;
		answer+="</CheckRequest>";
		SimpleAnswer(answer, *sock);
		delete sock;
	}
	else
		return -1;
	return 0;
}
