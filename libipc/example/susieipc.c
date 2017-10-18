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

