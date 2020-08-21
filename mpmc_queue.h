#ifndef __MPMC_QUEUE__
#define __MPMC_QUEUE__

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "src/core/lib/iomgr/executor/mpmcqueue.h"

class GrpcMpmcQueue
{
public:
	GrpcMpmcQueue(int capacity)
	{
		this->res_max = capacity;
		this->res_cnt = 0;
	}

	void enqueue(int element)
	{
		void *tmp = malloc(sizeof(int));
		memcpy(tmp, &element, sizeof(int));
		this->queue.Put(tmp);
		//this->queue.Put((void *)((unsigned long long)element));
	}

	int dequeue()
	{
		void *tmp = this->queue.Get();
		int ret = *(int *)tmp;
		free(tmp);
		return ret;
//		return (int)reinterpret_cast<unsigned long long>(this->queue.Get());
	}

	int size()
	{
		return this->queue.count();
		//return this->res_cnt;
	}
private:
	int res_max;
	int res_cnt;
	grpc_core::InfLenFIFOQueue queue;
};

#include "workflow_list.h"

struct __poller_queue_waiter
{
	struct list_head list;
	int res;
	pthread_cond_t cond;
};

#define WAITER_INITIALIZER (-1)

class MpmcQueue
{
public:
	MpmcQueue(int capacity)
	{
		this->res_max = capacity;
		this->res_cnt = 0;
		INIT_LIST_HEAD(&this->waiter_list);
	}

	void enqueue(int element)
	{
		return this->enqueue_basic(element);
		//return this->enqueue_cond_signal_outside(element);
	}

	int dequeue()
	{
		return this->dequeue_basic();
		//return this->dequeue_cond_signal_outside();
	}
	
	int size()
	{
		return this->res_cnt;
	}

	// signal outside the mutex; list_add_tail;
	void enqueue_basic(int element);
	int dequeue_basic();

	// signal outside the mutex; cond; list_add_head;
	void enqueue_cond_signal_outside(int element);
	int dequeue_cond_signal_outside();
	
private:
	int res_max;
	int res_cnt;
	struct list_head waiter_list;
	List<int> res_list;

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
};

////////impl

inline void MpmcQueue::enqueue_basic(int element)
{
	struct __poller_queue_waiter *waiter = NULL;
	struct list_head *pos;

	pthread_mutex_lock(&this->mutex);
	while (this->res_cnt > this->res_max - 1)// && !poller->stopping)
		pthread_cond_wait(&this->cond, &this->mutex);

	this->res_list.add_tail(element);
	this->res_cnt++;
	pos = this->waiter_list.next;
	if (pos != &this->waiter_list)
	{
		waiter = list_entry(pos, struct __poller_queue_waiter, list);
		list_del(pos);
		pthread_cond_signal(&waiter->cond);
		//waiter->cond = NULL;
		waiter->list.next = NULL;
	}

	pthread_mutex_unlock(&this->mutex);
//	if (waiter)
//		pthread_cond_signal(&waiter->cond);
}

inline int MpmcQueue::dequeue_basic()
{
	int ret;
	pthread_mutex_lock(&this->mutex);
	if (this->res_cnt == 0)// && !queue->nonblock)
	{

		struct __poller_queue_waiter waiter = {
			.list  =  { },
			.res   =  WAITER_INITIALIZER,
			.cond  =  PTHREAD_COND_INITIALIZER
		};

		do
		{
			list_add_tail(&waiter.list, &this->waiter_list);

			do
				pthread_cond_wait(&cond, &this->mutex);
			while (waiter.list.next); // ...

		} while (this->res_cnt == 0); // && !queue->nonblock)
	}

	if (this->res_cnt != 0)
	{
		this->res_list.get_head(ret);
		this->res_cnt--;
//		if (this->res_cnt-- > this->res_max - 1) // this is wrong
			pthread_cond_signal(&this->cond);
	}

   	pthread_mutex_unlock(&this->mutex);
	return ret;
}

inline void MpmcQueue::enqueue_cond_signal_outside(int element)
{
	struct __poller_queue_waiter *waiter;
	struct list_head *pos;

	pthread_mutex_lock(&this->mutex);
	while (this->res_cnt > this->res_max - 1)
		pthread_cond_wait(&this->cond, &this->mutex);

	pos = this->waiter_list.next;
	if (pos != &this->waiter_list)
	{
		waiter = list_entry(pos, struct __poller_queue_waiter, list);
		list_del(pos);
		waiter->res = 1412;
//		pthread_cond_signal(&waiter->cond);
	}
	else
	{
		this->res_list.add_tail(element);
		this->res_cnt++;
	}
	pthread_mutex_unlock(&this->mutex);
	pthread_cond_signal(&waiter->cond);
}

inline int MpmcQueue::dequeue_cond_signal_outside()
{
	int ret;
	pthread_mutex_lock(&this->mutex);
	if (this->res_cnt != 0)
	{
		this->res_list.get_head(ret);
		this->res_cnt--;
	}
	else
	{
		struct __poller_queue_waiter waiter = {
			.list  =  { },
			.res   =  WAITER_INITIALIZER,
			.cond  =  PTHREAD_COND_INITIALIZER
		};

		list_add_tail(&waiter.list, &this->waiter_list);
		do
			pthread_cond_wait(&waiter.cond, &this->mutex);
		while (waiter.res == WAITER_INITIALIZER);
	}
	pthread_mutex_unlock(&this->mutex);
	pthread_cond_signal(&this->cond);
	return ret;
}

#endif
