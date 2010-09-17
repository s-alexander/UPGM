#include <sys/syscall.h>
#include <cstdio>

#include "cserver.h"
#include "cpaycheckthreadparam.h"
#include "cpayguiderequest.h"
#include "cpaycheckcore.h"

paycheck::CServer::CServer():bonbon::CModule()
{
	killed=false;    
}

paycheck::CServer::~CServer()
{
    printf("SERVER DESTRUCTOR\n");
}

void paycheck::CServer::Kill()
{
    kill_lock.Lock();
    killed=true;
    kill_lock.UnLock();
}

bool paycheck::CServer::Killed()
{
    bool result=false;
    kill_lock.Lock();
    if (killed)
        result=true;
    kill_lock.UnLock();
    return result;
}
void *paycheck::CServer::Execute(bonbon::CThreadParam &params)
{

    paycheck::CPaycheckThreadParam &p=dynamic_cast<paycheck::CPaycheckThreadParam &>(params);
    paycheck::CSocket server;
    std::queue<paycheck::CSocket *> connections_store;

    std::queue<paycheck::CSocket *> active_sock_list;

   
    /* If listener doesnt' exist - create it. */
   
        paycheck::CSocket *socklistner=new paycheck::CSocket();
//        printf("Bind returns %i\n",socklistner->Bind(25001,"127.0.0.1"));
	printf("Bind returns %i\n",socklistner->Bind(p.paycheck_core->GetNetworkPort(),p.paycheck_core->GetNetworkDevice()));
        printf("Listen returns %i\n",socklistner->Listen());
        connections_store.push(socklistner);

       

    while (Killed()!=true)
    {
        //printf("we are here!!!!!!!!!!!\n");
	int delete_now=1;
            printf("connection number=%i\n", connections_store.size());
        std::queue<paycheck::CSocket *> active_sock_list;
        paycheck::CSocket::readmultiselect(connections_store, active_sock_list, 1, 0);
        if (active_sock_list.size()!=0)
            printf("Something happened\n");
        std::vector<paycheck::CSocket *>::iterator active_sock;

        //for (active_sock=active_sock_list.begin(); active_sock!=active_sock_list.end(); active_sock++)
        while(active_sock_list.size()>0)
        {
            paycheck::CSocket *sock=active_sock_list.front();
            active_sock_list.pop();

            if (sock!=NULL)
            {
                printf("we got a sock\n");
                if (sock==socklistner)
                {
                    /* New connection accepted */
                    printf("Acceptor detected\n");
                    int t=sock->Accept();
                    if (t>-1)
                    {
                        paycheck::CSocket *new_client=new paycheck::CSocket(t);
                        connections_store.push(new_client);
                    }
                    else
                    {
                        printf("No one to accept\n");
                        /* Something wrong with listener? */
                    }
                    connections_store.push(sock);
                }
                else
                {
                    /* New data from client */
                    std::string message;
                    int r=sock->Receive(message, 1024);
                    if (r>-1)
                    {
                	if (message.find(" HTTP")==std::string::npos || message.find("POST ")==std::string::npos)
        	        {
			    p.paycheck_core->GenericError(G_ERROR_WRONG_METHOD, *sock);
			}
			else
			{
			   delete_now=p.paycheck_core->Xml2PayguideFormat(message, *sock);
                        }
                    }
                    else
                    {
                        /* Connection lost */
                        printf("Connection lost.\n");
                    }
		    if (delete_now!=0) delete sock;
                }
            }
        }
    }
    
    while (connections_store.size()>0)
    {
        delete connections_store.front();
        connections_store.pop();
    }

/*    while (true)
    {
        p.in_manager->WaitForANewJob();
	working_lock.Lock();
	if (!working)
	{
		working_lock.UnLock();
		return NULL;
	}
	working_lock.UnLock();
        
        p.in_manager->LockQueue();
        
        int queue_size=p.in_manager->GetJobsCount();
        if (queue_size>512)
            queue_size=512;
        
        //sock_list.clear();
        active_sock_list.clear();
        
        for (int i=0; i<queue_size; i++)
        {
            paycheck::CSocket *sock=(paycheck::CSocket *)p.in_manager->GetJob();
            sock_list.push_back(sock);
        }
        
        p.in_manager->UnLockQueue();
        
        paycheck::readmultiselect(sock_list, active_sock_list, 5, 0);
	
        std::vector<paycheck::CSocket *>::iterator active_sock;
        
        for (active_sock=active_sock_list.begin(); active_sock<active_sock_list.end(); active_sock++)
        {
            paycheck::CSocket *sock=*active_sock;
            
            if (sock!=NULL)
            {
                std::string xml_message;
                bool reading=true;
                bool get_message=false;
                bool connection_lost=false;

                printf("we are here! sock addr=%p\n", sock);
                int r=sock->Receive(xml_message, 1024);
                if (r==-1 || xml_message.length()>POST_MESSAGE_LIMIT)
                {
		    p.paycheck_core->GenericError(G_ERROR_WRONG_METHOD, *sock);
                    printf("Connection lost\n");
                    reading=false;
                    connection_lost=true;
                }
                else
                {
                    printf("%i byte(s) readed; xml_message=[%s]\n", r, xml_message.c_str());
		    
                    size_t start=xml_message.find("POST ");
                    if (start!=std::string::npos)
                    {
                        size_t end=xml_message.find(" HTTP");
                        if (end!=std::string::npos)
                        {
                            get_message=true;
                            reading=false;
                        }
                    }
		    else
		    {
                	start=xml_message.find(" HTTP");
                	if (start!=std::string::npos)
        	        {
				p.paycheck_core->GenericError(G_ERROR_WRONG_METHOD, *sock);
                        	get_message=false;
                        	reading=false;
				connection_lost=true;
				
                	}
		    
		    }
                }

                if (get_message)
                {

                    int request=p.paycheck_core->Xml2PayguideFormat(xml_message, *sock);

                    if (request==0)
                    {
                        //Data received in wrong format

//                        sock->ForceDown();
//                        delete sock;
                    }
                    else
                    {
                        //Push request to payguide
//			p.check_manager->PushJob(*request);
//			printf("PUSH\n");
                        //delete request;
                    }
                }
                else
                {
                    //Wrong request or connection error
                    if (connection_lost)
                    {
                        //Connection lost
                       sock->ForceDown();
                        delete sock;
			printf("socket removed\n");
                    }
                    else
                    {
                        //Still not all data received - push to queueslist
			
                        p.in_manager->PushJob(*sock);
                    }
                }
            }
            else
            {
                printf("There is an error in programm somewhere: socket is NULL\n");
            }
        }
	
	working_lock.Lock();
	if (!working)
	{
		working_lock.UnLock();
		return NULL;
	}
	working_lock.UnLock();
    }*/


    printf("Thread %li returning.\n",syscall(SYS_gettid));
    return NULL;
}

