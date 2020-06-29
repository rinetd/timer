#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "timer.h"

#define MAXFDS 128

static int SetNonBlock (int fd)
{
    int flags = fcntl (fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (-1 == fcntl (fd, F_SETFL, flags)) {
		printf("%s,%d errors:SetNonBlock fcntl \n",__FUNCTION__,__LINE__);
        return -1;
    }
    return 0;
}

static void* timer_run(void* arg)
{
	char buf[128];

	timer_server_t *handle = (timer_server_t *)arg;
	if (NULL == handle) return NULL;

	if (0 != pthread_mutex_init(&handle->mutex_, NULL)) {
		printf("%s,%d errors: pthread_mutex_init error \n",__FUNCTION__,__LINE__);
		return NULL;
	}

    for (; handle->active_ ; ) {
        struct epoll_event events[MAXFDS] ;
        int nfds = epoll_wait (handle->epoll_fd_, events, MAXFDS, -1);
        for (int i = 0; i < nfds; ++i)
        {
    		timer *t = NULL;
            timer_func_t cb = NULL;
			pthread_mutex_lock(&handle->mutex_);
            HASH_FIND_INT(handle->timers_, &events[i].data.fd, t);
            while (read(events[i].data.fd, buf, 128) > 0);
            if (t) {
                cb = t->cb_;
            }
			pthread_mutex_unlock(&handle->mutex_);

			if (0 == handle->active_)
				break;

			if (cb && t->count_ != 0) {
				cb(t->argv);
				if (t->count_ > 0)
			        t->count_--;
			}
        }
    }
	pthread_mutex_lock(&handle->mutex_);
    close(handle->epoll_fd_);
    handle->epoll_fd_ = -1;
    for(int i = 0; i <= handle->fd_max_; ++i) {
		timer *t = NULL;
		HASH_FIND_INT(handle->timers_, &i, t);
		if(t != NULL) {
			HASH_DEL(handle->timers_, t);
			free(t);
		}
	}
	pthread_mutex_unlock(&handle->mutex_);
	pthread_mutex_destroy(&handle->mutex_);
	free(handle);
    handle = NULL;

	printf("%s,%d Standard simple_timer thread exit  \n",__FUNCTION__,__LINE__);
    return NULL ;
}


timer_server_t * timer_server_init(int max_num /* 最大定时器个数 */)
{
	timer_server_t *handle = (timer_server_t *)malloc(sizeof(timer_server_t));
	if (NULL == handle) {
		fprintf(stderr, "timer_server create failed\n");
		return NULL;
	}

	handle->init_success_ = 1;
	handle->active_ = 1;
	handle->fd_max_ = -1;

    handle->epoll_fd_ = epoll_create(MAXFDS);
    if (-1 == handle->epoll_fd_) {
		fprintf(stderr, "epoll_create failed\n");
		goto timer_epoll_create_failed;
    }

    pthread_create(&handle->tid_, NULL, timer_run, (void *)handle);

    return handle;

timer_epoll_create_failed:
	free(handle);
	handle = NULL;
	printf("t init error\n");
timer_handle_failed:

	return NULL;
}


void timer_server_uninit(timer_server_t *timer_handle)
{
	timer_handle->active_ = 0;
	pthread_join(timer_handle->tid_, NULL);
}

int timer_server_addtimer(timer_server_t *timer_handle, timer *tt)
{
	int nRet;

	timer *t = (timer *)malloc(sizeof(timer));
	if (NULL == t) {
		fprintf(stderr, "malloc t failed\n");
		return -1;
	}

	t->count_ = tt->count_;
	t->timer_internal_ = tt->timer_internal_;
	t->cb_ = tt->cb_;
	t->argv = tt->argv;
	t->fd = tt->fd = timerfd_create(CLOCK_REALTIME, 0);
    if (-1 == t->fd) {
		printf("%s,%d errors: timer_create error m_nTimerfd=%d \n",__FUNCTION__,__LINE__,t->fd); 
		return -1;
	}
    SetNonBlock(t->fd);
    if (timer_handle->fd_max_ < t->fd) timer_handle->fd_max_ = t->fd;

	printf("%s,%d Information: add timerfd=%d \n",__FUNCTION__,__LINE__, t->fd);

    struct epoll_event ev;
    ev.data.fd = t->fd;
    ev.events = EPOLLIN | EPOLLET;

	pthread_mutex_lock(&timer_handle->mutex_);
	struct itimerspec ptime_internal ;
    ptime_internal.it_value.tv_sec = (int)t->timer_internal_;
    ptime_internal.it_value.tv_nsec = (t->timer_internal_ - (int) t->timer_internal_)*1000000000;
    if(0 != t->count_) {
        ptime_internal.it_interval.tv_sec = ptime_internal.it_value.tv_sec;
        ptime_internal.it_interval.tv_nsec = ptime_internal.it_value.tv_nsec;
		nRet = timerfd_settime(t->fd, 0, &ptime_internal, NULL);
    	if (-1 == nRet) {
			fprintf(stderr, "%s,%d  errors timerfd_settime\n",__FUNCTION__,__LINE__);
    	}
    }

	if (-1 == epoll_ctl(timer_handle->epoll_fd_, EPOLL_CTL_ADD, t->fd, &ev)) {
		printf("%s,%d errors epoll_ctl add_timer \n",__FUNCTION__,__LINE__);
		nRet = -1;
	}
	else {
		int fd = t->fd;
		HASH_ADD_INT(timer_handle->timers_, fd, t);
	}
	pthread_mutex_unlock(&timer_handle->mutex_);

	return nRet;
}

void timer_server_deltimer(timer_server_t *timer_handle, int timer_fd)
{
	printf("%s,%d Information: start del timerfd=%d \n",__FUNCTION__,__LINE__, timer_fd);

	timer *t = NULL;
	
	pthread_mutex_lock(&timer_handle->mutex_);
	HASH_FIND_INT(timer_handle->timers_, &timer_fd, t);
	if (t) {
		printf("delete fd:%d\n", timer_fd);
		struct epoll_event ev;
		ev.data.fd = timer_fd;
		ev.events = EPOLLIN | EPOLLET;
		epoll_ctl(timer_handle->epoll_fd_, EPOLL_CTL_DEL, timer_fd, &ev);
		close(timer_fd);
		HASH_DEL(timer_handle->timers_,t);
		free(t);
	}
	pthread_mutex_unlock(&timer_handle->mutex_);
}

// 定义全局 
static timer_server_t *g_handle=NULL;

void * timer_new(){
	if (g_handle)
	{
		printf("t server has init\n");
		// 已经初始化过
		return g_handle;
	}
	g_handle = timer_server_init(MAXFDS);

	return g_handle;
}

void * timer_add(unsigned int ms,void (*cb)(void *arg),void * arg){

	timer_new();
	timer *t = (timer *)malloc(sizeof(timer));
	t->count_ = -1;
	t->timer_internal_ = ms/1000.0;
	t->cb_ = (timer_func_t)cb;
	t->argv = arg;
	// t.fd = 0;
	printf("t fd %d\n",t->fd);
	if (g_handle)
	{
		timer_server_addtimer(g_handle, t);
	}

	return t;
}

int timer_del(void* id){
	timer *t =(timer *)id;
	timer_server_deltimer(g_handle, t->fd);	
}
void timer_stop(){

	// t *t, *tmp;
	// HASH_ITER(hh, g_handle->timers_, t, tmp) {
    //     printf("%d (id %s)\n", t->fd, t->argv);
    //     HASH_DEL(g_handle->timers_,t);
    //     free(t);
    // }
	
	timer_server_uninit(g_handle);
	g_handle = NULL;

}
