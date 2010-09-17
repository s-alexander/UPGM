#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>

#include "csocket.h"

static int get_hostaddr(const char *name);

paycheck::CSocket::CSocket():bonbon::CJob()
{
    	bzero (&addr, sizeof (addr));
	listener=-1;
	socket_id=-1;
	error=0;
	addr.sin_family = AF_INET;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	rtv.tv_sec = 0;
	rtv.tv_usec = 0;

	wtv.tv_sec = 0;
	wtv.tv_usec = 0;

}

paycheck::CSocket::CSocket(int sock_id):bonbon::CJob()
{
    	bzero (&addr, sizeof (addr));
	listener=-1;
	error=0;
	addr.sin_family = AF_INET;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	rtv.tv_sec = 0;
	rtv.tv_usec = 0;

	wtv.tv_sec = 0;
	wtv.tv_usec = 0;
        socket_id=sock_id;
}

paycheck::CSocket::~CSocket()
{
    	if (listener!=-1)
		close(listener);
	if (socket_id!=-1)
		close(socket_id);
}

void paycheck::CSocket::ForceDown()
{
    lock.Lock();
    if (socket_id!=-1)
    {
	shutdown(socket_id,SHUT_RDWR);
        printf("socket_id=%i\n",socket_id);
	close(socket_id);
	socket_id=-1;
    }
    lock.UnLock();
}

int paycheck::CSocket::GetFD()
{
    lock.Lock();
    int result=socket_id;
    if (listener!=-1)
        result=listener;
    lock.UnLock();
    return result;
}

int paycheck::CSocket::Bind(int port, const char *interface)
{
    lock.Lock();

    listener = socket (AF_INET, SOCK_STREAM, 0);
    int val=-1;
    if (setsockopt (listener, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof (val))!=0)
    {
        perror("setsockopt SO_REUSEADDR");
	lock.UnLock();
	return -1;
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
        lock.UnLock();
        return -1;
    }
    error=0;
    lock.UnLock();
    return 0;
}


int paycheck::CSocket::Listen()
{
        lock.Lock();
	if (listen (listener, 20) != 0)
	{
		//ERROR
		error=CRS_ERR_LISTEN;
                lock.UnLock();
		return -1;
	}
	error=0;
	if (fcntl(listener, F_SETFL, O_NONBLOCK)!=0)
	{
		perror("fcntl O_NONBLOCK");
	}
        lock.UnLock();
	return 0;
}

int paycheck::CSocket::Accept()
{
	int size=sizeof (addr);
        int result=-1;
	lock.Lock();
	error=0;
        FD_ZERO(&rfds);
	FD_SET(listener, &rfds);
        rtv.tv_sec = 1;
	rtv.tv_usec = 0;
        socket_id = accept (listener, (struct sockaddr *) &addr, (socklen_t *) & size);
        if (socket_id==-1)
            error=CRS_ERR_ACCEPT;
        else
            result=socket_id;
	lock.UnLock();
	return result;
}

int paycheck::CSocket::Receive(std::string &buff, int max_size)
{
    return Receive(buff, max_size, -1);
}

int paycheck::CSocket::Receive(std::string &buff, int max_size, float timeout)
{
    lock.Lock();
    timeout=0;
    int result=0;
    if (socket_id==-1)
    {
            error=CRS_ERR_LOST;
            lock.UnLock();
            return -1;
    }

    int sock_id=socket_id;
    error=0;
    int retval=0;
    if (timeout==-1)
        retval=1;
    else
    {
        rtv.tv_sec = 1;
        rtv.tv_usec = 0;


        FD_ZERO(&rfds);
        FD_SET(socket_id, &rfds);


        retval=select(socket_id+1, &rfds, NULL, NULL, &rtv);
        printf("socket id=%i select=%i\n", socket_id, retval);
        if (retval<0)
        {

    //        if (errno==EBADF) printf("В одном из наборов находится неверный файловый дескриптор.\n");
    //        if (errno==EINTR) printf("Был пойман незаблокированный сигнал.\n");
    //        if (errno==EINVAL) printf("n отрицательно.\n");
    //        if (errno==ENOMEM) printf("Функция select не смогла выделить участок памяти для внутренних таблиц.\n");
    //        error=CRS_ERR_LOST;
            lock.UnLock();
            return -1;
        }
    }

    if (retval==1)
    {
        char msg_part[1024];
        int err=recv(sock_id, msg_part,1023,0);
        if (err>0)
        {
            buff.append(msg_part,err);
	    result+=err;

        }
	else
	    result=-1;
    }
    lock.UnLock();

    return result;
}

int paycheck::CSocket::Send(const char *data, size_t size)
{
	if (data==NULL) return 0;
	lock.Lock();
	FD_ZERO(&wfds);
	if (socket_id==-1)
	{
		lock.UnLock();
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
		lock.UnLock();
		return -1;
	}
	int r=send(socket_id,data,size,0);
	lock.UnLock();
	return r;
}

int paycheck::CSocket::Send(const char *message)
{
    return this->Send(message, strlen(message));
}

int paycheck::CSocket::Send(std::string &message)
{
    return this->Send(message.c_str(), message.length());
}

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


int paycheck::CSocket::readmultiselect(std::queue<paycheck::CSocket *> &in_s_list, std::queue<paycheck::CSocket *> &out_s_list, int sec, int usec)
{
    fd_set rfds, wfds;
    struct timeval my_tv;
    my_tv.tv_sec = sec; my_tv.tv_usec = usec;
    int result=0;

    int max_fd = -1;
    FD_ZERO(&rfds);

    std::queue<paycheck::CSocket *> cache_list;

    while (in_s_list.size()>0)
    {
        paycheck::CSocket *pc_clnt=in_s_list.front();
        in_s_list.pop();
        if (pc_clnt!=NULL)
        {
            int socket_id=pc_clnt->GetFD();
            if (socket_id>=0)
            {
                if(socket_id > max_fd)
                    max_fd = socket_id;

                FD_SET(socket_id, &rfds);
            }
            cache_list.push(pc_clnt);
        }
    }

    printf ("select returns %i, max sock=%i\n", select(max_fd + 1, &rfds, NULL, NULL, &my_tv), max_fd);

    //for (it=in_s_list.begin(); it<in_s_list.end(); it++)
    while (cache_list.size()>0)
    {
        paycheck::CSocket *pc_clnt=cache_list.front();
        cache_list.pop();

        if (pc_clnt!=NULL)
        {
            int socket_id=pc_clnt->GetFD();
            if (socket_id>=0)
            {
                if (FD_ISSET(socket_id, &rfds))
                {
                    out_s_list.push(pc_clnt);
                    result++;
                }
                else
                    in_s_list.push(pc_clnt);
            }
        }
    }

    return result;
}

/*
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
*/
