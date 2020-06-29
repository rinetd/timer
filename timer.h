/*
 * @Author: 李守磊
 * @Date: 2020-06-27 09:52:59
 * @LastEditTime: 2020-06-29 18:31:34
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /timer.h
 */ 

#ifndef _LINUX_TIMER_H
#define _LINUX_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif


  
#include <stdlib.h>
#include <pthread.h>
#include "uthash.h"
// return: -1: auto free timer;
typedef int (*timer_func_t)(void *u);

// void* timer_add(int ms, timer_func_t *func, void *u);

int timer_del(void* id);

void (*cb)(void *arg);
// typedef int(*timer_func_t)(void *);
typedef struct _ctimer{
	int                 fd;
	int 				count_;		
    double              timer_internal_;
    timer_func_t        cb_;
    void*               argv;
    UT_hash_handle 		hh;
}timer;

typedef struct
{
	int 				init_success_;
	int          		active_;
	pthread_t			tid_;
	int             	epoll_fd_;
	int 				fd_max_;
	timer*		 		timers_;   // hash 入口指针
	pthread_mutex_t 	mutex_;
}timer_server_t;

timer_server_t * timer_server_init(int max_num /* 最大定时器个数 */);
int timer_server_addtimer(timer_server_t *timer_handle, timer *timer);
void timer_server_deltimer(timer_server_t *timer_handle, int timer_fd);
void timer_server_uninit(timer_server_t *timer_handle);

void * timer_add(unsigned int ms,void (*cb)(void *),void * arg);
void timer_stop();


#ifdef __cplusplus
}
#endif

#endif