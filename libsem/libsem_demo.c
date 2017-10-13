/*
需求:
	银行办理业务规则,顾客需要先到取号机取号,然后到服务窗口办理业务.
	目前,已知只有一个取号机,2个服务窗口. 取到号的顾客才可以办理业务,
	请使用信号量得方法编程实现该情景.
分析:
	只有一个取号机,可以设置一个值为1的信号量.两个服务窗口,可以设置一
	个值为2的信号量. 编写一个共用取号线程和一个公用服务线程.作为共享
	资源,用信号量来控制共享资源的访问
*/
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>
#include "libpublic.h"
#include "libsem.h"

#define MAX_NUM		20			/*顾客数量*/
pthread_rwlock_t	lock_machine;
sem_t	sem_machine;
sem_t	sem_service;
int 	customernums[MAX_NUM] = {0};

void* machine_proc(void *arg)
{
	
}
void* service_proc(void *arg)
{
	int cstmid;

	cstmid = *(int *)arg;
	semaphore_p(&sem_service,WAIT_FOREVER,0);
	log_printf(LOG_FAIL, "customer %d in service!\n",cstmid);
	sleep(2);
	semaphore_v(&sem_service);
	log_printf(LOG_FAIL, "customer %d service over!\n",cstmid);
}
int main(int argc,const char *argv[]) 
{
	int 	i,arg;
	pthread_t	customerid[MAX_NUM];

	if(OK != create_semaphore(&sem_service,1))
	{
		log_printf(LOG_FAIL, "main create sem_machine fail!\n");
		return FAIL;
	}
	for(i = 0;i < MAX_NUM;i++)
	{
		if(	-1 == pthread_create(&(customerid[i]),
							NULL,
							service_proc,
							(void *)&i))
		{
			log_printf(LOG_FAIL, "pthread_create fail id = %d!\n",i);
			continue;
		}
		sleep(1);
	}
	while(1);
	return OK;
}
