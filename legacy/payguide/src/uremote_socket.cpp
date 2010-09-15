#include "uremote_socket.h"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>

static int get_hostaddr(const char *name)
{
    struct hostent *he;
    int             res = -1;
    int             a1,a2,a3,a4;
    
    if (sscanf(name,"%d.%d.%d.%d",&a1,&a2,&a3,&a4) == 4)
        res = inet_addr(name);
    else
    {
        he = gethostbyname(name);
        if (he)
            memcpy(&res , he->h_addr , he->h_length);
    }
    return res;

}

CRemoteSocket::CRemoteSocket()
{
//	socket_id[i]=-1;
		
	bzero (&addr, sizeof (addr));
	listener=0;
	error=0;
	addr.sin_family = AF_INET;
	
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	
	rtv.tv_sec = 0;
	rtv.tv_usec = 0;

	wtv.tv_sec = 0;
	wtv.tv_usec = 0;
	sem_init(&lock,0,1);

}

CRemoteSocket::~CRemoteSocket()
{
	if (listener!=0)
		close(listener);
}

int CRemoteSocket::Bind(int port, const char *interface)
{
	sem_wait(&lock);
	listener = socket (AF_INET, SOCK_STREAM, 0);
	int val=-1;
	if (setsockopt (listener, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof (val))!=0)
	{
			perror("setsockopt SO_REUSEADDR");
	}
		
		
	addr.sin_port = htons (port);
	
	if (interface!=NULL)
	{
		addr.sin_addr.s_addr = (get_hostaddr(interface));
	}
	else
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	
	if (bind (listener, (struct sockaddr *) &addr, sizeof (addr)) != 0)
	{
		//ERROR
		error=CRS_ERR_BIND;
		sem_post(&lock);
		return -1;
	}
	error=0;
	sem_post(&lock);
	return 0;

}


int CRemoteSocket::Listen()
{
	sem_wait(&lock);
	if (listen (listener, 20) != 0)
	{
		//ERROR
		error=CRS_ERR_LISTEN;
		sem_post(&lock);
		return -1;
	}
	error=0;
	sem_post(&lock);
	return 0;

}

CSocketRW *CRemoteSocket::Accept()
{
	CSocketRW *result=NULL;
	int size=sizeof (addr);

	sem_wait(&lock);
	error=0;
	if (fcntl(listener, F_SETFL, O_NONBLOCK)!=0)
	{
		perror("fcntl O_NONBLOCK");
	}

	socket_id = accept (listener, (struct sockaddr *) &addr, (socklen_t *) & size);
	if (socket_id==-1)error=CRS_ERR_ACCEPT;
	else
	{
		result=new CSocketRW(socket_id);
	}
	//printf("NEW socket id=%i\n", socket_id);
	sem_post(&lock);
	return result;

}

CSocketRW *CRemoteSocket::Connect(const char *ip, int port, int to)
{
	sem_wait(&lock);
	CSocketRW *result=NULL;
	socket_id = socket (AF_INET, SOCK_STREAM, 0);
	addr.sin_port = htons (port);
	inet_aton (ip, &addr.sin_addr);
	error=0;
	int flags = fcntl(socket_id, F_GETFL, 0);
	if(fcntl(socket_id, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		/*
		* Не получилось...
		*/
		error=CRS_ERR_CONNECT;
		sem_post(&lock);
		return result;
	}
	else
	{
		int r=connect (socket_id, (struct sockaddr *) &addr, sizeof (addr));
		if (r!=0)
		{
			if(errno == EINPROGRESS)
			{
				//Соединение еще не установлено
			}
		        else
			{
				//Произошла ошибка
				error=CRS_ERR_CONNECT;
				sem_post(&lock);
				return result;
			}
		}
		fd_set wfds;
		struct timeval my_tv;
		
		//Ожидаю соединения timeout секунд
		my_tv.tv_sec = to; my_tv.tv_usec = 0;


		int max_fd = -1;

		FD_ZERO(&rfds); 
		FD_ZERO(&wfds);
		FD_SET(socket_id, &wfds);
		FD_SET(socket_id, &rfds);
		if(socket_id > max_fd) max_fd = socket_id;

		select(max_fd + 1, &rfds, &wfds, NULL, &my_tv);
		
		if(FD_ISSET(socket_id, &wfds) || FD_ISSET(socket_id, &rfds))
		{
			socklen_t err_len;
			int err=0;

			err_len = sizeof(err);
			if(getsockopt(socket_id, SOL_SOCKET, SO_ERROR, &err, &err_len) < 0 || err != 0)
			{
				error=CRS_ERR_CONNECT;
				sem_post(&lock);
				return result;
			}
			else
			{
				error=0;
				result=new CSocketRW(socket_id);
				sem_post(&lock);
				return result;
			}
		}
		else
		{
			error=CRS_ERR_CONNECT;
			sem_post(&lock);
			return result;
		}
		result=new CSocketRW(socket_id);
	}
	sem_post(&lock);
	return result;
}

int CRemoteSocket::Err()
{
	sem_wait(&lock);
	int result=error;
	sem_post(&lock);
	return result;
}

CSocketRW::CSocketRW(int socket)
{
	socket_id=socket;
	sem_init(&lock,0,1);
	timeout=0;
	reading=false;
	read_step=0;
	
	write_posssible=0;
	read_posssible=0;

	read_buff_ptr=read_buff;
	read_buff_left=UREMOTE_SOCKET_PACK_SIZE;
		
	message_buff_ptr=message_buff;
	message_buff_left=UREMOTE_SOCKET_PACK_SIZE;
	new_data=0;
	auth=0;

}

void CSocketRW::Authorize(int val)
{
	sem_wait(&lock);
	auth=val;
	sem_post(&lock);
}

int CSocketRW::Authorized()
{
	sem_wait(&lock);
	int result=auth;
	sem_post(&lock);
	return result;
}


CSocketRW::~CSocketRW()
{
//	socket_id=socket;
}

bool CSocketRW::Reading()
{
	sem_wait(&lock);
	bool result=reading;
	sem_post(&lock);
	return result;
}

int CSocketRW::TimeOut()
{
	sem_wait(&lock);
	timeout++;
	int result=timeout;
	sem_post(&lock);
	return result;
}

void CSocketRW::WritePossible(int val)
{
	sem_wait(&lock);
	write_posssible=val;
	sem_post(&lock);
}

void CSocketRW::ReadPossible(int val)
{
	sem_wait(&lock);
	read_posssible=val;
	sem_post(&lock);
}

int CSocketRW::WriteIsPossible()
{
	sem_wait(&lock);
	int result=write_posssible;
	sem_post(&lock);
	return result;
}

int CSocketRW::ReadIsPossible()
{
	sem_wait(&lock);
	int result=read_posssible;
	sem_post(&lock);
	return result;
}




int CSocketRW::Send(const char *data, unsigned short int size)
{
	if (data==NULL) return 0;
	char buff[UREMOTE_SOCKET_PACK_SIZE];
//	char buff2[UREMOTE_SOCKET_PACK_SIZE*2];
	char *point=buff+1;
	if (size>UREMOTE_SOCKET_PACK_SIZE-4) size=UREMOTE_SOCKET_PACK_SIZE-4;
	
	buff[0]=buff[0] | 0xFF;
	memcpy(point, &size, sizeof(size));
	point+=sizeof(size);
	memcpy(point, data, size);
	
	sem_wait(&lock);
	FD_ZERO(&wfds);
//	printf("We are here! %i\n", socket_id);
	if (socket_id==-1)
	{
		sem_post(&lock);
		return -1;
	}
	FD_SET(socket_id, &wfds);
	
	wtv.tv_sec = 0;
	wtv.tv_usec = 10000;
	
	int retval=select(socket_id+1, NULL, &wfds, NULL, &wtv);
	if (retval <=0)
	{
		//ERROR OR TIMEOUT - NO DATA SEND
		error=CRS_ERR_SEND;
		sem_post(&lock);
		return -1;
	}
	int r=send(socket_id,buff,(int)size+3,0);
		
	if (r>0)
	{
		error=0;
		if (abs(r)==((int)size+3)) {sem_post(&lock);return 0;}
	}
	error=CRS_ERR_SEND;
	sem_post(&lock);
	return -1;
}


void CSocketRW::Down()
{
	sem_wait(&lock);
	if (socket_id!=-1)
	{
		shutdown(socket_id,SHUT_RDWR);
		close(socket_id);
		socket_id=-1;
	}
	sem_post(&lock);
}

void CRemoteSocket::ListenerDown()
{
    
    shutdown(listener,SHUT_RDWR);
    close(listener);
    listener=0;
    
}

int CSocketRW::Receive(char *buff, int buff_size, int to, int uto)
{
	if (buff_size>UREMOTE_SOCKET_PACK_SIZE)
		buff_size=UREMOTE_SOCKET_PACK_SIZE;
	int err=1;

	sem_wait(&lock);
	timeout=0;
	if (socket_id==-1)
	{
		error=CRS_ERR_LOST;
		ReadReset();
		sem_post(&lock);
		return -1;
	}

	int sock_id=socket_id;
	error=0;
	int retval=0;
	{
		//Минимальная скорость - 0.5 байт/сек
		if (new_data==0)
		{
			rtv.tv_sec = to;
			rtv.tv_usec = uto;
			
		
			FD_ZERO(&rfds);
			FD_SET(socket_id, &rfds);
		
			//printf("socket id=%i\n", socket_id);
			retval=select(socket_id+1, &rfds, NULL, NULL, &rtv);
		}
		
		if (retval <0)
		{
			
//			if (errno==EBADF) printf("В одном из наборов находится неверный файловый дескриптор.\n");
//			if (errno==EINTR) printf("Был пойман незаблокированный сигнал.\n");
//			if (errno==EINVAL) printf("n отрицательно.\n");
//			if (errno==ENOMEM) printf("Функция select не смогла выделить участок памяти для внутренних таблиц.\n");
			error=CRS_ERR_LOST;
			ReadReset();
			sem_post(&lock);
			return -1;
		}
		
		
		if (retval>0 || new_data>0)
		{
			reading=true;
			int read_request=3;
			int package_left=package_len-(message_buff_ptr-message_buff)+3;
			
			if (read_step==2)
			{
				read_request=package_left;
			}
			
			if (new_data==0)
			{
				err=recv(sock_id, read_buff_ptr,read_request,0);
//				printf("* %i bytes readed from recv\n",err);
			}
			else
			{
				err=new_data;
//				printf("* %i bytes virt readed from recv\n",err);
			}
			new_data=0;
			
			if (err<=0 || read_buff_left-err<0)
			{
				error=CRS_ERR_LOST;
				ReadReset();
				sem_post(&lock);
				return -1;
			}
			
			read_buff_left-=err;
			int read_buff_count=err;
			

			
//			printf("%i bytes true readed, read_buff fulled by %i bytes, step=%i\n", err, read_buff_count, read_step);
			if (read_step==0)
			{
//				printf("step %i, read_buff[0]=%i, read_buff[0]&0xFF=%i\n", read_step,read_buff[0],(read_buff[0]) & 0xFF);
				if ((unsigned char)read_buff[0]==0xFF)
				{
					read_step=1;
					memcpy(message_buff_ptr, read_buff_ptr, 1);
					read_buff_ptr++;
					message_buff_ptr++;
					read_buff_count--;
				}
				else
				{
					error=CRS_ERR_SEQ;
					ReadReset();
					sem_post(&lock);
					return -1;
				}
			
			}
			
			if (read_step==2)
				package_left=package_len-(message_buff_ptr-message_buff)+3;
			
			if (read_step==1)
			{

				char *rbptr=read_buff_ptr;

				if (read_buff_ptr-read_buff>=2 || read_buff_count>=2)
				{
					if (read_buff_ptr-read_buff>=2)
						rbptr=read_buff_ptr-1;
						
					memcpy(&package_len, rbptr, 2);
					memcpy(message_buff_ptr, rbptr, 2);
					message_buff_ptr+=2;
					if (!(read_buff_ptr-read_buff>=2))
					{
						read_buff_ptr+=2;	
						read_buff_count-=2;
					}
					else
					{
						read_buff_ptr+=1;	
						read_buff_count-=1;
					}
					read_buff_count-=2;
					read_buff_count+=(read_buff_ptr)-rbptr;
					package_left=package_len-(message_buff_ptr-message_buff)+3;
					read_step=2;
				}
				else
					read_buff_ptr+=read_buff_count;
			}
			
			if (read_step==2 && read_buff_count>0)
			{
				if (package_left>read_buff_count)
				{
					memcpy(message_buff_ptr, read_buff_ptr, read_buff_count);
					message_buff_ptr+=read_buff_count;
					read_buff_ptr=read_buff;
					read_buff_left=UREMOTE_SOCKET_PACK_SIZE;

					sem_post(&lock);
					return 0;
				}
				else
				{
					memcpy(message_buff_ptr, read_buff_ptr, package_left);
					message_buff_ptr+=package_left;
					int new_msg_len=err-(read_buff_ptr-read_buff+package_left);
					if (new_msg_len>0)
					{
						new_data=0;
					}
					else
					{
						new_data=0;
					}
					read_buff_ptr+=err;
					if (buff_size>package_len)
					{
						memcpy(buff, message_buff+3, package_len);
						ReadReset();
						sem_post(&lock);
						read_step=0;
						
						return package_len;
					}
					else
					{
						memcpy(buff, message_buff+3, buff_size);
						ReadReset();
						sem_post(&lock);
						return buff_size;
					}
				}
			}
		}
	}
	error=0;
	sem_post(&lock);
	return 0;
	
}

int CSocketRW::Err()
{
	sem_wait(&lock);
	int result=error;
	sem_post(&lock);
	return result;
}

int CSocketRW::GetFD()
{
	sem_wait(&lock);
	int result=socket_id;
	sem_post(&lock);
	return result;
}

void CSocketRW::ReadReset()
{
	reading=false;
	read_buff_ptr=read_buff;
	read_buff_left=UREMOTE_SOCKET_PACK_SIZE;
		
	message_buff_ptr=message_buff;
	message_buff_left=UREMOTE_SOCKET_PACK_SIZE;
	timeout=0;
}


int readmultiselect(CUlist<CPayguideClient> *clientlist, int sec, int usec)
{
	if (clientlist==NULL)
		return 0;
	unsigned int l=clientlist->GetLen();
	if (l<1)
		return 0;

	fd_set rfds, wfds;
	struct timeval my_tv;
	my_tv.tv_sec = sec; my_tv.tv_usec = usec;
	int result=0;

	int max_fd = -1;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds); 
	clientlist->ResetCursor();
	for (unsigned int a=0; a<l; a++)
	{
		CPayguideClient *pg_clnt=clientlist->GetNext();
		if (pg_clnt!=NULL)
		{
			CSocketRW *sock=pg_clnt->GetSocket();
			if (sock!=NULL)
			{
				int socket_id=sock->GetFD();
				if (socket_id>=0)
				{
					if(socket_id > max_fd) max_fd = socket_id;
			
					FD_SET(socket_id, &rfds);
					FD_SET(socket_id, &wfds);
				}
			}
		}
	}
	

	select(max_fd + 1, &rfds, NULL, NULL, &my_tv);
	
	clientlist->ResetCursor();
	for (unsigned int a=0; a<l; a++)
	{
		
		CPayguideClient *pg_clnt=clientlist->GetNext();
		if (pg_clnt!=NULL)
		{
			CSocketRW *sock=pg_clnt->GetSocket();
		
			if (sock!=NULL)
			{
				int socket_id=sock->GetFD();
				if (socket_id>=0)
				{

					if (FD_ISSET(socket_id, &rfds))
					{
						sock->ReadPossible(1);
						result++;
					}
					else
						sock->ReadPossible(0);
				}
			}
		}
	}

	return result;
}

int writemultiselect(CUlist<CPayguideClient> *clientlist, int sec, int usec)
{
	unsigned int l=clientlist->GetLen();
	if (l<1)
		return 0;

	fd_set rfds, wfds;
	struct timeval my_tv;
	my_tv.tv_sec = sec; my_tv.tv_usec = usec;


	int max_fd = -1;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds); 
	clientlist->ResetCursor();
	for (unsigned int a=0; a<l; a++)
	{
		CPayguideClient *pg_clnt=clientlist->GetNext();
		if (pg_clnt!=NULL)
		{
			CSocketRW *sock=pg_clnt->GetSocket();
			if (sock!=NULL)
			{
				int socket_id=sock->GetFD();
				if (socket_id>=0)
				{
					if(socket_id > max_fd) max_fd = socket_id;
			
					FD_SET(socket_id, &rfds);
					FD_SET(socket_id, &wfds);
				}
			}
		}
	}
	

	select(max_fd + 1, NULL, &wfds, NULL, &my_tv);
	
	clientlist->ResetCursor();
	for (unsigned int a=0; a<l; a++)
	{
		CPayguideClient *pg_clnt=clientlist->GetNext();
		if (pg_clnt!=NULL)
		{
			CSocketRW *sock=pg_clnt->GetSocket();

			if (sock!=NULL)
			{
				int socket_id=sock->GetFD();

				if (socket_id>=0)
				{
					if (FD_ISSET(socket_id, &wfds))
						sock->WritePossible(1);
					else
						sock->WritePossible(0);
				}
			}
		}
	}

	return 0;
}
