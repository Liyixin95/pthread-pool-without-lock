#include "pool.h"

sigset_t mask;
thread_pool *pool;

int j;

static void sig_handler()
{
	return;
}

void *routine(void *arg)
{
	int signo;
	int i;
	
	struct thread_work *worker = (struct thread_work *)arg;
	struct task *job;
		
	while(1)
	{		
		#ifdef    DEBUG
			printf("%lu is waiting\n",pthread_self());
		#endif
		while(worker -> waiting_tasks == 0 && !worker -> shutdown)
		{
			sigwait(&mask,&signo);
			if(signo == SIGUSR1 && worker -> waiting_tasks > 0)
				break;
		}
		#ifdef    DEBUG
			printf("%lu start to work\n",pthread_self());
			printf("waitingtask = %d,head = %d,tail = %d\n",worker -> waiting_tasks,worker -> head,worker -> tail);
		#endif
		
		if(worker -> waiting_tasks == 0 && worker -> shutdown == true)
		{			
			pthread_exit(0);                                      //如果线程池关闭就退出
		}
		
		i = worker -> head;
		job = &worker -> task_queue[i];		//从任务队列中取出一个任务
	
		if(worker -> head >= MAX_THREAD_WAITING_TASKS-1)                         
		{
			worker -> head = 0;
		}
		else
		{
			worker -> head ++;
		}
		worker -> waiting_tasks --;
		pool -> waiting_tasks --;
		
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);     //执行任务代码之前要设置取消属性
		(job -> task)(job -> arg);                               //执行任务代码
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
		
	}
}

bool init_pool(unsigned int number)
{
	thread_work *tmp;
	signal(SIGUSR1,sig_handler);                                 //捕捉信号SIGUSR1
	sigemptyset(&mask);
	sigaddset(&mask,SIGUSR1);                                    //将SIGUSR1加入信号集
	
	pthread_sigmask(SIG_BLOCK,&mask,NULL);                       //屏蔽SIGUSR1信号，子线程会继承信号掩码
		
	pool -> worker = malloc(sizeof(struct thread_work) * MAX_ACTIVED_THREADS);
	pool -> waiting_tasks = 0;
	pool -> actived_threads = number;	
	int i;
	for(i = 0;i<MAX_ACTIVED_THREADS;i++)
	{
		tmp = pool -> worker + i;
		tmp -> head = 0;
		tmp -> tail = 0;		
		
		tmp -> waiting_tasks = 0;
		tmp -> shutdown = false;
		
		if(pthread_create(&(tmp -> tid),NULL,routine,(void *)tmp) != 0)
		{
			printf("create erroe\n");
			return false;
		}
		#ifdef    DEBUG
			printf("%lu is created\n",tmp -> tid);
		#endif
	}
	
	j = 0;
		
	return true;
}

bool add_task(void *(*task)(void *arg),void *arg)
{
	int i;
	thread_work *tmp;
	
	if(pool -> waiting_tasks >= MAX_WAITING_TASKS)        //如果任务数量过多则直接返回
	{
		#ifdef    DEBUG
			printf("too many tasks\n");
		#endif
		return false;
	}	
	
	//min = pool -> worker -> waiting_tasks;                     //找到最短的任务队列
/* 	j = 0;
	for(i = 0;i<MAX_ACTIVED_THREADS;i++)
	{
		tmp = pool -> worker + i;
		if(tmp -> waiting_tasks < min)
		{
			min = tmp -> waiting_tasks;
			j = i;
		}
	} */
	tmp = pool -> worker + j;                                    //轮转调度
	j++;
	if(j ==  MAX_ACTIVED_THREADS - 1)
		j = 0;	
	
	i = tmp -> tail;
	memset(&(tmp -> task_queue[i]),0,sizeof(struct task));
	tmp -> task_queue[i].task = task;
	tmp -> task_queue[i].arg = arg;	
	if(tmp -> tail >= MAX_THREAD_WAITING_TASKS -1)                         //队尾插入
	{
		tmp -> tail = 0;
	}
	else
	{
		tmp -> tail ++;
	}
		
	tmp -> waiting_tasks++;
	pool -> waiting_tasks++;
	
	pthread_kill(tmp -> tid,SIGUSR1);	
	return true;	
}

























