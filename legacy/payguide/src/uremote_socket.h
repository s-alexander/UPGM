#ifndef __Uremote_socket__
#define __Uremote_socket__
#include <sys/socket.h>
#include <sys/un.h>
#include <semaphore.h>
#include <string>
#include <vector>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include "ulist.h"

#define CRS_ERR_BIND 1
#define CRS_ERR_LISTEN 2
#define CRS_ERR_ACCEPT 3
#define CRS_ERR_CONNECT 4
#define CRS_ERR_LOST 5
#define CRS_ERR_SEND 6
#define CRS_ERR_SEQ 7

#define UREMOTE_SOCKET_PACK_SIZE 5024


using std::string;

class CSocketRW
{
	public:
		CSocketRW(int socket_id);
		~CSocketRW();
		int Send(const char *msg, unsigned short int size);
		int Err();
		void Down();
//		char *Receive(int *l, int sec, int usec);
		int Receive(char *buff, int buff_size, int sec, int usec);
		int GetFD();
		void WritePossible(int val);
		void ReadPossible(int val);
		int WriteIsPossible();
		int ReadIsPossible();
		bool Reading();
		int TimeOut();
		void Authorize(int val);
		int Authorized();
	private:
		void ReadReset();
		string msg;
		string str_pack;
		
		int socket_id;
//		sockaddr_in addr;
//		sem_t read_lock;
		
		fd_set rfds;
		fd_set wfds;
		struct timeval rtv;
		struct timeval wtv;
		sem_t lock;
		int error;
		int write_posssible;
		int read_posssible;
		int timeout;
		bool reading;
		
		int read_step;
		
		char read_buff[UREMOTE_SOCKET_PACK_SIZE+1];
		char *read_buff_ptr;
		int read_buff_left;
		
		char message_buff[UREMOTE_SOCKET_PACK_SIZE+1];
		char *message_buff_ptr;
		int message_buff_left;
		
		unsigned short package_len;
		
		int new_data;
		int auth;
		
		
};


class CRemoteSocket
{
	public:
		CRemoteSocket();
		~CRemoteSocket();
		int Bind(int port, const char *i);
		int Listen();
		CSocketRW *Accept();
		CSocketRW *Connect(const char *ip, int port, int timeout);
		//void Down();
		void ListenerDown();
//		char *Receive(int *l, int sec, int usec);
//		int Send(const char *msg, short int size);
		int Err();
	private:
		string msg;
		string str_pack;
		
		//CSocketRW sockets[CRS_MAX_CON];
		int socket_id;
		int listener;
		sockaddr_in addr;
		sem_t read_lock;
		
		fd_set rfds;
		fd_set wfds;
		struct timeval rtv;
		struct timeval wtv;
		int error;
		sem_t lock;

};

class CPayguideClient
{
	public:
		CPayguideClient(){delivery=0; sock=NULL; username="<unauth>";};
		~CPayguideClient()
		{
			if (sock!=NULL)
			{
				sock->Down();
				delete sock;
			}
		};
		void LinkSocket(CSocketRW *s)
		{
			if (s!=NULL && sock!=NULL)
			{
				sock->Down();
				delete sock;
			}
			sock=s;
		};
		CSocketRW *GetSocket(){return sock;};
		void AddToDelivery(){delivery=1;};
		void RemoveFromDelivery(){delivery=0;};
		int InDelivery(){return delivery;};
		void Authorize(int a_level, const char *login){auth_level=a_level;if (login!=NULL){username=login;}};
		const char *GetName(){return username.c_str();};
	protected:
		int delivery;
		CSocketRW *sock;
		std::string username;
		int auth_level;
};

int readmultiselect(CUlist<CPayguideClient> *clientlist, int sec, int usec);
int writemultiselect(CUlist<CPayguideClient> *clientlist, int sec, int usec);

#endif
