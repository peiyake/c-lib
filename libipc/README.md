# c-lib
##项目需求
+	编写一些接口,隐藏细节,实现进程间通信.
+	相同接口,实现跨网络进程间通信
##接口列表
+	`int init_ipc(const char*app_name,ipc_callback func)`
 	
		/*
		 * 函数名称:init_ipc
		 * 函数功能:使用本libipc必须首先调用此函数,用于
		 * 初始化一些通信接口
		 * 参数说明:
		 * app_name 	进程名,一个字符串
		 * func  		收到消息后的回调函数
		 * 返回值  :
		 * 0 			初始化失败
		 * 1 			初始化成功
		 * */
+	`int ipc_send(unsigned char *data,unsigned int datalen,const char*receiver)`

		/*
		 * 函数名称:ipc_send
		 * 函数功能:消息发送接口函数
		 * 参数说明:
		 * data 		消息体指针
		 * datalen  	消息体长度
		 * receiver		接收者
		 * 返回值  :
		 * 0 			发送失败
		 * 1 			发送成功
		 * */

##使用方法
1.	`git clone git@github.com:peiyake/c-lib.git`
2.	`cd libipc`
3.	`cd src && make 	//编译libipc.a静态库等待使用`
4.	`cd example && make //编译例程`
5.	`./susie  			//运行susie`
6.	`./piak				//运行piak`
7.	两个终端互相输入,查看进程通信效果

##配置文件
**libipc.conf**

+	创建配置文件`/etc/libipc.conf`,也可以修改源码修改配置文件存储路径.
+	配置文件格式,例如:

		#name	port	flag
		susie	8090	1
		piak	9090	1


name:进程名,这个就是init_ipc时注册的进程名

port:跨网络的进程需要配置所用的端口号

flag:是否本机进程,1 本机进程;0 非本机进程

##例程
+	**本机进程susie:**

		#include <stdio.h>
		#include <string.h>
		#include "libipc.h"
		#include "libpublic.h"
		void susie_proc(unsigned char *msg)
		{
			data_header_t *head;
			char recbuf[512];	
			
			head = (data_header_t*)msg;
			memset(recbuf,0,sizeof(recbuf));
			memcpy(recbuf,msg + sizeof(data_header_t),head->datalen);
			
			fprintf(stdout,"%-10s:%s\n",head->sender,recbuf);
		}
		int main(int argc, char *argv[]) 
		{
			int ret;
			char buf[256];
			ret = init_ipc("susie",susie_proc);
			if(ret != OK)
			{
				log_printf(LOG_MSG,"proc[%s]init_ipc fail and exit\n",argv[0]);
				return 0;
			}
			memset(buf,0,256);
			while(NULL != fgets(buf,sizeof(buf),stdin))
			{
				ipc_send(buf,strlen(buf)-1,"piak");
				memset(buf,0,256);
			}
		}

+	**本机进程piak:**

		#include <stdio.h>
		#include <string.h>
		#include "libipc.h"
		#include "libpublic.h"
		void piak_proc(unsigned char *msg)
		{
			data_header_t *head;
			char recbuf[512];	
			
			head = (data_header_t*)msg;
			memset(recbuf,0,sizeof(recbuf));
			memcpy(recbuf,msg + sizeof(data_header_t),head->datalen);
			
			fprintf(stdout,"%-10s:%s\n",head->sender,recbuf);
		}
		int main(int argc, char *argv[])
		{
			int ret;
			char buf[256];
			ret = init_ipc("piak",piak_proc);
			if(ret != OK)
			{
				log_printf(LOG_MSG,"proc[%s]init_ipc fail and exit\n",argv[0]);
				return 0;
			}
			memset(buf,0,256);
			while(NULL != fgets(buf,sizeof(buf),stdin))
			{
				ipc_send(buf,strlen(buf)-1,"susie");
				memset(buf,0,256);
			}
		}


