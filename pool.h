#ifndef __POOL_H
#define __POOL_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define     MAX_ACTIVED_THREADS        30
#define     MAX_WAITING_TASKS          300
#define     MAX_THREAD_WAITING_TASKS   10

//#define     DEBUG                      1

typedef struct task
{
	void *(*task)(void *arg);
	void *arg;
	
}task;

typedef struct thread_work
{
	pthread_t tid;
	task task_queue[MAX_THREAD_WAITING_TASKS];                //数组实现循环链表
	int head;                                                 //队头
	int tail;                                                 //队尾
	unsigned waiting_tasks;                                   //单个线程的等待任务数目
	bool shutdown;
}thread_work;

typedef struct thread_pool
{	
	thread_work *worker;	
	unsigned waiting_tasks;             //线程池的等待任务数目
	unsigned actived_threads;		
}thread_pool;

bool init_pool(unsigned int number);
bool add_task(void *(*task)(void *arg),void *arg);
void *routine(void *arg);




#endif  /* __POOL_H */