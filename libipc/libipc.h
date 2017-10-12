#ifndef __LIB_IPC__
#define __LIB_IPC__

#define OK				1
#define	FAIL			0
#define IS_LOCAL_PROC	1
#define IS_OTHER_PROC	0

#define  IPC_FAIL    		5
#define  IPC_ERROR    		4
#define  IPC_WARNING  		3
#define  IPC_INFO     		2
#define  IPC_MSG   			1
#define ipc_printf(level, format, arg...) do{\
		syslog(level,format, ##arg);	\
}while(0)

typedef struct 
{
	int srv_sockfd;
	int cli_sockfd;
}unix_sock_t;
typedef struct
{
	char srv_path[64];
	char cli_path[64];
}unix_path_t;
typedef struct proc
{
	char appname[32];		/*进程名*/
	unsigned int flag;		/*1:本机进程     0:非本机进程*/
	unsigned int port;		/*进程监听端口*/
	unsigned int id;
	unsigned int byuse;
	
	unix_path_t unix_path;
	unix_sock_t unix_sock;
}proc_t;
typedef struct 
{
	char sender[32];
	unsigned int datalen;
}data_header_t;
typedef struct
{
	char version[32];
	unsigned int length;
}ipc_header_t;
typedef void (*ipc_callback)(unsigned char*);

#define IPC_CONFIG_FILE	"/etc/libipc.conf"
#define UNIX_ROOT_PATH 	"/tmp/sun_path/"

#define MAX_APP_NUM	32
#define MAX_LINE_SIZE	1024
#define MSG_BUF_SIZE	48*1024
#define MSG_HEADER_SIZE	64
#define MAX_MSGBUF_SIZE	(MSG_BUF_SIZE+MSG_HEADER_SIZE)
#endif
