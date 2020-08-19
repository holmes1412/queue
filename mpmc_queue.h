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
		this->queue.Put((void *)((unsigned long long)element));
	}

	int dequeue()
	{
		return (int)reinterpret_cast<unsigned long long>(this->queue.Get());
	}

	int size()
	{
		return this->queue.count();
		return this->res_cnt;
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
	pthread_cond_t cond;
};

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
			//pthread_cond_signal(&waiter->cond);
			//waiter->cond = NULL;
			waiter->list.next = NULL;
		}

		pthread_mutex_unlock(&this->mutex);
		if (waiter)
			pthread_cond_signal(&waiter->cond);
	}

	int dequeue()
	{
		int ret;
		pthread_mutex_lock(&this->mutex);
 		if (this->res_cnt == 0)// && !queue->nonblock)
		{

			struct __poller_queue_waiter waiter = {
				.list  =  { },
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
//			if (this->res_cnt-- > this->res_max - 1) // this is wrong
				pthread_cond_signal(&this->cond);
		}

   		pthread_mutex_unlock(&this->mutex);
		return ret;
	}

	int size()
	{
		return this->res_cnt;
	}

private:
	int res_max;
	int res_cnt;
	struct list_head waiter_list;
	List<int> res_list;

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
};

#endif
