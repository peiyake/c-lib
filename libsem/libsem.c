#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include "libsem.h"
#include "libpublic.h"

int create_semaphore(sem_t *sem,unsigned int sem_num)
{
	if(0 != sem_init(sem,0,sem_num))
	{
		log_printf(LOG_FAIL, "create_semaphore sem_init fail : %s", strerror(errno));
		return FAIL;	
	}
	return OK;
		
}
int semaphore_p(sem_t *sem,int type,time_t time)
{
	int ret;
	struct timespec time_s;
	
	if(time != 0)
	{
		time_s.tv_sec = time;
		time_s.tv_nsec = 0;
	}
	switch(type)
	{
		case NO_WAIT:
			ret = sem_trywait(sem);
			break;
		case WAIT_FOREVER:
			ret = sem_wait(sem);
			break;
		case WAIT_TIME:
			ret = sem_timedwait(sem,&time_s);
		break;
	}
	return (ret == 0) ? OK : FAIL;
}
int semaphore_v(sem_t *sem)
{
	int ret;
	ret = sem_post(sem);
	if(0 != ret)
	{
		log_printf(LOG_FAIL, "semaphore_v  sem_post fail\n");
		return FAIL;
	}
	return OK;
}
