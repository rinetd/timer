/*
 * @Author: your name
 * @Date: 2020-06-27 09:52:59
 * @LastEditTime: 2020-06-29 18:34:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /clearn/simple-timer-for-c-language/main.c
 */ 
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "timer.h"

int timer_cb1(void *timer)
{
	// if (timer && timer->argv) {
	// 	int *user_data = timer->argv;
	// 	printf("call back data:%d\n", *user_data);
	// }
		printf("timer_cb1 %s   \n",(char *)timer);
	return 0;
}

int timer_cb2(timer *timer)
{
	if (timer && timer->argv) {
		int *user_data = timer->argv;
		printf("call back data:%d\n", *user_data);
	}
	return 0;
}

void time_callback(void * arg ){
	printf("%s\n",(char *)arg);
}

int main(int argc, char *argv[])
{
	// timer_server_t *timer_handle = timer_server_init(1024);
	// if (NULL == timer_handle) {
	// 	fprintf(stderr, "timer_server_init failed\n");
	// 	return -1;
	// }

	// timer timer1;
	// timer1.count_ = -1;
	// timer1.timer_internal_ = 0.5;
	// timer1.cb_ = timer_cb1;
	// int *user_data1 = (int *)malloc(sizeof(int));
	// *user_data1 = 100;
	// timer1.argv = user_data1;
	// timer_server_addtimer(timer_handle, &timer1);

	// timer timer2;
	// timer2.count_ = -1;
	// timer2.timer_internal_ = 0.5;
	// timer2.cb_ = timer_cb2;
	// int *user_data2 = (int *)malloc(sizeof(int));
	// *user_data2 = 10;
	// timer2.argv = user_data2;
	// timer_server_addtimer(timer_handle, &timer2);

	// sleep(10);

	// timer_server_deltimer(timer_handle, timer1.fd);
	// timer_server_deltimer(timer_handle, timer2.fd);
	// timer_server_uninit(timer_handle);

	char *str ="nihao ";
	char *str1 ="测试2 ";

	void * t1=timer_add(1000,time_callback,str);
	void * t2=timer_add(1500,time_callback,str1);
	getchar();
	timer_del(t2);
	getchar();
	timer_stop();
	return 0;
}

