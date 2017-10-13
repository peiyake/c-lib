#ifndef __LIB_SEM_H__
#define __LIB_SEM_H__

#define	NO_WAIT				1
#define	WAIT_FOREVER		2
#define	WAIT_TIME			3

extern	int create_semaphore(sem_t *sem,unsigned int sem_num);
extern 	int semaphore_p(sem_t *sem,int type,time_t time);
extern 	int semaphore_v(sem_t *sem);
#endif
