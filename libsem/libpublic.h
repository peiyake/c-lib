#ifndef __LIB_PUBLIC_H__
#define __LIB_PUBLIC_H__

#define FAIL	0
#define OK		1

#define  LOG_FAIL    		5
#define  LOG_ERROR    		4
#define  LOG_WARNING  		3
#define  LOG_INFO     		2
#define  LOG_MSG   			1
#define log_printf(level, format, arg...) do{\
		syslog(level,format, ##arg);	\
}while(0)

#endif
