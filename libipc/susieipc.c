#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>       
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
//#include <linux/in.h>
#include "libipc.h"

static proc_t proc[MAX_APP_NUM];
static proc_t myproc;
static fd_set groupfd;
static unsigned char msgbuf[MAX_MSGBUF_SIZE];
static char version[32]={"libipc-1.0"};
static ipc_callback callback_func = NULL;

int is_annotation(char *data)
{
	int i = 0;
	do{
		if(data[i] != ' ')
		{
			if(data[i] == '#')
				return OK;
			else
				return FAIL;
		}
		if(MAX_LINE_SIZE == i)
			break;
	}while(i++);
}
int ipc_proc_init(const char*app_name)
{
	FILE *pfile = NULL;
	char buf[MAX_LINE_SIZE];
	char cmd[64];
	int i = 0;
	proc_t *p = NULL;
	
	memset(proc,0,sizeof(proc_t)*MAX_APP_NUM);
	
	/*read config file*/
	pfile = fopen(IPC_CONFIG_FILE,"r");
	if(NULL == pfile)
	{
		ipc_printf(IPC_ERROR,"ipc_proc_init fail, fopen config file fail!\n");
		return FAIL;
	}
	memset(buf,0,sizeof(buf));
	while(NULL != fgets(buf,MAX_LINE_SIZE,pfile))
	{
		if(i > MAX_APP_NUM)
		{
			ipc_printf(IPC_WARNING,"ipc_proc_init fail, config too many apps:max[%d]\n",MAX_APP_NUM);
			return FAIL;
		}
		if((buf[MAX_LINE_SIZE-1] != '\0') && (buf[MAX_LINE_SIZE-1] != '\n'))
		{
			ipc_printf(IPC_ERROR,"ipc_proc_init fail, Beyond the size in config file line:%d\n",i);
			return FAIL;
		}
		if(OK == is_annotation(buf))
			continue;
		if(3 != sscanf(buf,"%s%d%d",proc[i].appname,&(proc[i].port),&(proc[i].flag)))
		{
			ipc_printf(IPC_ERROR,"ipc_proc_init fail,Wrong config style in line %d\n",i);
			return FAIL;
		}
		proc[i].id = i+1;
		proc[i].byuse = 1;
	
		memset(buf,0,sizeof(buf));
		i++;
	}
	
	ipc_printf(IPC_MSG,"-appname--port--flag(1/0)--srv_path--cli_path\n",i);
	
	/*make sunpath which will by used by unix domain socket*/
	memset(cmd,0,64);
	sprintf(cmd,"mkdir -p %s",UNIX_ROOT_PATH);
	system(cmd);
	for(i = 0;i < MAX_APP_NUM;i++)
	{
		if(proc[i].byuse && (IS_LOCAL_PROC == proc[i].flag))
		{
			sprintf(proc[i].unix_path.srv_path,"%s%s.%d",UNIX_ROOT_PATH,proc[i].appname,proc[i].id);
			sprintf(proc[i].unix_path.cli_path,"%s%s.%d.cli",UNIX_ROOT_PATH,proc[i].appname,proc[i].id);
			ipc_printf(IPC_MSG,"-%s--%d--%d--%s--%s\n",
					proc[i].appname,
					proc[i].port,
					proc[i].flag,
					proc[i].unix_path.srv_path,
					proc[i].unix_path.cli_path);
		}
	}
	return OK;
}
int creat_unix_srv_udpsock(proc_t *p)
{
	int loop;
	int sock;
	struct sockaddr_un seraddr;
	memset(&seraddr,0,sizeof(seraddr));
	sock = socket(AF_LOCAL,SOCK_DGRAM,0);
	if(-1 == sock)
	{
		ipc_printf(IPC_FAIL,"creat_unix_srv_udpsock socket fail!\n");
		return FAIL;
	}
	unlink(p->unix_path.srv_path);
	seraddr.sun_family = AF_LOCAL;
	strncpy(seraddr.sun_path,p->unix_path.srv_path,sizeof(seraddr.sun_path));
	if(-1 == bind(sock,(struct sockaddr*)&seraddr,sizeof(seraddr)))
	{
		ipc_printf(IPC_FAIL,"creat_unix_srv_udpsock bind fail!\n");
		close(sock);
		return FAIL;
	}
	p->unix_sock.srv_sockfd = sock;
	ipc_printf(IPC_FAIL,"creat_unix_srv_udpsock success!\n");
	return OK;
}
int creat_unix_cli_udpsock(proc_t *p)
{
	int sock;
	struct sockaddr_un cliaddr;
	sock = socket(AF_LOCAL,SOCK_DGRAM,0);
	if(-1 == sock)
	{
		ipc_printf(IPC_FAIL,"creat_unix_cli_udpsock socket fail!\n");
		return FAIL;
	}
	memset(&cliaddr,0,sizeof(cliaddr));
	unlink(p->unix_path.cli_path);
	cliaddr.sun_family = AF_LOCAL;
	strcpy(cliaddr.sun_path,p->unix_path.cli_path);
	if(-1 == bind(sock,(struct sockaddr*)&cliaddr,sizeof(cliaddr)))
	{
		ipc_printf(IPC_FAIL,"creat_unix_cli_udpsock bind fail!\n");
		close(sock);
		return FAIL;
	}
	p->unix_sock.cli_sockfd = sock;
	ipc_printf(IPC_FAIL,"creat_unix_cli_udpsock success!\n");
	return OK;
}
void ipc_msg_handle(unsigned char *data,unsigned int len)
{
	unsigned int offset;
	unsigned char *msg;
	unsigned int headlen;
	ipc_header_t 	*ipc_header;
	ipc_header_t	ipc_head;
	data_header_t	*data_header;
	data_header_t	data_head;
	if(NULL == data)
	{
		ipc_printf(IPC_ERROR,"ipc_msg_handle recv msg is NULL!\n");
		return;
	}
	if(len < (sizeof(ipc_header_t) + sizeof(data_header_t)))
	{
		ipc_printf(IPC_ERROR,"ipc_msg_handle msglen too short!\n");
		return;
	}
	offset = 0;
	memset(&ipc_head,0,sizeof(ipc_header_t));
	memset(&data_head,0,sizeof(data_header_t));
	
	ipc_header = (ipc_header_t*)data;
	ipc_head.length = ntohl(ipc_header->length);
	memcpy(ipc_head.version,ipc_header->version,sizeof(ipc_header->version));

	data_header = (data_header_t *)(data + sizeof(ipc_header_t));
	memcpy(data_head.sender,data_header->sender,sizeof(data_header->sender));
	data_head.datalen = ntohl(data_header->datalen);
	
	if(strncmp(ipc_head.version,version,strlen(version)))
	{
		ipc_printf(IPC_ERROR,"ipc_msg_handle  sender version[%s] != currunt version[%s] \n",
			ipc_head.version,version);
		return;
	}
	if(len != ipc_head.length)
	{
		ipc_printf(IPC_ERROR,"ipc_msg_handle  sender length[%d],recive msglen[%d]\n",
			ipc_head.length,len);
		return;
	}
	ipc_printf(IPC_MSG,"ipc_msg_handle  receive message from:%s,ipc_version:%s,len:%d,headlen:%d,datalen:%d\n",
				data_head.sender,
				ipc_head.version,
				len,
				ipc_head.length,
				data_head.datalen);

	msg = (unsigned char*)malloc(len - sizeof(ipc_header_t));
	if(NULL == msg)
	{
		ipc_printf(IPC_ERROR,"ipc_msg_handle  malloc fail !\n");
		return;
	}
	memcpy(msg,&data_head,sizeof(data_header_t));
	offset = sizeof(data_header_t);

	headlen = sizeof(ipc_header_t) + sizeof(data_header_t);
	memcpy(msg+offset,data+headlen,data_head.datalen);
	
	callback_func(msg);
	free(msg);
	return;
}
void ipc_recvmsg(int fd)
{
	struct sockaddr fromaddr;
	socklen_t addrlen;
	unsigned int recvbytecount;
	
	memset(&fromaddr,0,sizeof(fromaddr));
	memset(msgbuf,0,sizeof(msgbuf));
	addrlen = sizeof(fromaddr);
	recvbytecount = recvfrom(fd,
							msgbuf,
							MAX_MSGBUF_SIZE,
							0,
							(struct sockaddr*)&fromaddr,
							(socklen_t*)&addrlen);
	if(-1 == recvbytecount)
	{
		ipc_printf(IPC_ERROR,"ipc_recvmsg recvfrom got error !\n");
		return;
	}
	ipc_msg_handle(msgbuf,recvbytecount);
}
void *ipc_thread(void *arg)
{
	
	ipc_printf(IPC_MSG,"ipc_thread start !\n");
	FD_ZERO (&groupfd);
	static int serfd;
	serfd = myproc.unix_sock.srv_sockfd;
	FD_SET(serfd, &groupfd);
	while(1)
	{
		if(select(FD_SETSIZE, &groupfd, NULL, NULL, NULL) == -1)
		{
			ipc_printf(IPC_ERROR,"ipc_thread select got error!\n");
			continue;
		}

		if(FD_ISSET(serfd, &groupfd))
		{
			//FD_CLR (serfd, &groupfd);
            ipc_recvmsg(serfd);
		}
	}
	
	return NULL;
}
int init_ipc(const char*app_name,ipc_callback func)
{
	int	ret;
	int loop;
	proc_t *procinfo;
	pthread_t unix_thread;
	
	if(NULL == app_name)
	{
		ipc_printf(IPC_ERROR,"init_ipc fail, arg appname is NULL!\n");
		return FAIL;
	}
	ret = ipc_proc_init(app_name);
	if(FAIL == ret)
	{
		ipc_printf(IPC_ERROR,"init_ipc , ipc_proc_init fail\n");
		return FAIL;
	}
	for(loop = 0;loop < MAX_APP_NUM;loop++)
	{
		procinfo = &(proc[loop]);
		if(strncmp(app_name,procinfo->appname,sizeof(app_name)))
			continue;
		break;
	}
	if(loop == MAX_APP_NUM)
	{
		ipc_printf(IPC_FAIL,"init_ipc , %s proc not registered in "IPC_CONFIG_FILE"\n",app_name);
		return FAIL;
	}
	
	/*创建UINX域套接字用于本机进程间通信*/
	if(creat_unix_srv_udpsock(procinfo) &&
			creat_unix_cli_udpsock(procinfo))
	{
		ipc_printf(IPC_FAIL,"init_ipc,create unix udpsock  success!\n");
	}else
	{
		ipc_printf(IPC_MSG,"init_ipc,create unix udpsock  Fail!\n");
		return FAIL;
	}
	
	memset(&myproc,0,sizeof(myproc));
	memcpy(&myproc,procinfo,sizeof(proc_t));
	if (0 != pthread_create((pthread_t*)&unix_thread,
							(pthread_attr_t*)NULL,
							(void*)ipc_thread,
							(void*)NULL))
	{
		ipc_printf(IPC_FAIL,"init_ipc,pthread_create fail!\n");
		close(myproc.unix_sock.cli_sockfd);
		close(myproc.unix_sock.srv_sockfd);
		return FAIL;
	}
	callback_func = func;
	ipc_printf(IPC_MSG,"init_ipc success!\n");
	return OK;
}
int get_proc_info(proc_t *dst,const char *appname)
{
	int loop;
	for(loop = 0;loop<MAX_APP_NUM;loop++)
	{
		if(!strncmp(appname,proc[loop].appname,sizeof(appname)))
		{
			memcpy(dst,&(proc[loop]),sizeof(proc_t));
			return OK;
		}
	}
	return FAIL;
}
int ipc_send(unsigned char *data,unsigned int datalen,const char*receiver)
{
	ipc_header_t 	ipc_header;
	data_header_t 	data_header;
	struct sockaddr_un seraddr;
	unsigned char *msg = NULL;
	unsigned int msglen;
	unsigned int offset;
	unsigned int addrlen,ret;
	proc_t dst_proc;
	/**
	msg格式:
			类别			元素			长度			说明
			----------------------------------------
			ipc消息头		version		32		ipc版本 例如:"libipc-1.0"
						length		4		发送的总的UDP报文长度
			数据头			sender		32		发送进程名
						len			4		发送的有效数据长度
			数据			data		*		有效数据
	*/
	if(FAIL == get_proc_info(&dst_proc,receiver))
	{
		ipc_printf(IPC_MSG,"ipc_send can't find the dst proc %s!\n",receiver);
		return FAIL;
	}
	offset = 0;
	msglen =sizeof(ipc_header_t) + sizeof(data_header_t) + datalen;
	msg = (unsigned char *)malloc(msglen);

	memset(&ipc_header,0,sizeof(ipc_header));
	memset(&data_header,0,sizeof(data_header));
	memset(msg,0,msglen);
	
	strncpy(ipc_header.version,version,sizeof(version));
	ipc_header.length = htonl(msglen);
	strncpy(data_header.sender,myproc.appname,sizeof(myproc.appname));
	data_header.datalen = htonl(datalen);

	memcpy(msg,&ipc_header,sizeof(ipc_header));
	offset += sizeof(ipc_header);
	memcpy(msg+offset,&data_header,sizeof(data_header));
	offset += sizeof(data_header);
	memcpy(msg+offset,data,datalen);
	
	memset(&seraddr,0,sizeof(seraddr));
	seraddr.sun_family = AF_LOCAL;
	strncpy(seraddr.sun_path,dst_proc.unix_path.srv_path,sizeof(seraddr.sun_path));
	addrlen = sizeof(seraddr);
	ret = sendto(myproc.unix_sock.cli_sockfd,
			(unsigned char*)msg,
			(size_t)msglen,
			0,
			(struct sockaddr*)&seraddr,
			(socklen_t)addrlen);
	if(-1 == ret)
	{
		ipc_printf(IPC_FAIL,"ipc_send sendto got error!\n");
		free(msg);
		return FAIL;
	}
	ipc_printf(IPC_MSG,"ipc_send send message to %s!\n",receiver);
	free(msg);
	return OK;
}
void susie_proc(unsigned char *msg)
{
	data_header_t *head;
	char recbuf[512];	
	
	head = (data_header_t*)msg;
	memset(recbuf,0,sizeof(recbuf));
	memcpy(recbuf,msg + sizeof(data_header_t),head->datalen);
	
	fprintf(stdout,"susie_proc recvmsg from [%s] len[%d] msg[%s]\n",
			head->sender,head->datalen,recbuf);
}
int main(int argc, char *argv[]) 
{
	int ret;
	char buf[256];
	ret = init_ipc("susie",susie_proc);
	if(ret != OK)
	{
		ipc_printf(IPC_MSG,"proc[%s]init_ipc fail and exit\n",argv[0]);
		return 0;
	}
	memset(buf,0,256);
	while(NULL != fgets(buf,sizeof(buf),stdin))
	{
		ipc_send(buf,strlen(buf),"piak");
		memset(buf,0,256);
	}
}

