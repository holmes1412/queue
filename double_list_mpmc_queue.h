#ifndef __DOUBLE_LIST_MPMC_QUEUE__
#define __DOUBLE_LIST_MPMC_QUEUE__

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include "list.h" // demo list
#include "workflow_list.h"

template<typename T>
class DoubleListMpmcQueue
{
private:
	struct __queue_waiter
	{
		struct list_head list;
		bool flag;
		pthread_cond_t cond;
	};

public:
	DoubleListMpmcQueue(int capacity)
	{
		this->res_max = capacity;
		this->put_res_cnt = 0;
		this->get_res_cnt = 0;
//		INIT_LIST_HEAD(&this->put_waiter_list);
		INIT_LIST_HEAD(&this->get_waiter_list);
	}

	void enqueue(T element)
	{
		struct __queue_waiter *waiter = NULL;
		struct list_head *pos;

		pthread_mutex_lock(&this->put_mutex);
		while (this->put_res_cnt >= this->res_max)
			pthread_cond_wait(&this->put_cond, &this->put_mutex);

		pos = this->get_waiter_list.next;
		// somebody is waiting
		if (pos != &this->get_waiter_list)
		{
			waiter = list_entry(pos, struct __queue_waiter ,list);
			list_del(pos);
			waiter->flag = true;
		}
		else
		{
			this->put_list.add_tail(element);
			this->put_res_cnt++;
		}

		pthread_mutex_unlock(&this->put_mutex);
		if (waiter)
			pthread_cond_signal(&waiter->cond);
	}

	T dequeue()
	{
		T ret;

		pthread_mutex_lock(&this->get_mutex);
		if (!this->get_list.empty() || this->swap_list() > 0)
		{
			this->get_list.get_head(ret);
			this->get_res_cnt--;
		}

		pthread_mutex_unlock(&this->get_mutex);
		return ret;
	}

	int swap_list()
	{
		pthread_mutex_lock(&this->put_mutex);
		if (this->put_res_cnt == 0)
		{
			struct __queue_waiter waiter = {
				.list   =   {},
				.flag   =   false,
				.cond   =   PTHREAD_COND_INITIALIZER
			};

			list_add(&waiter.list, &this->get_waiter_list);
			do
				pthread_cond_wait(&waiter.cond, &this->put_mutex);
			while (waiter.flag == false);
		}
		else
		{
			this->get_res_cnt = this->put_res_cnt;

			if (this->get_res_cnt > this->res_max - 1)
				pthread_cond_broadcast(&this->put_cond); // why broadcast

			this->get_list = std::move(this->put_list);
			this->put_res_cnt = 0;
		}
		pthread_mutex_unlock(&this->put_mutex);
		return this->get_res_cnt;
/*
		pthread_mutex_lock(&this->put_mutex);

		while (this->put_res_cnt == 0)
			pthread_cond_wait(&get_cond, &put_mutex);

		this->get_res_cnt = this->put_res_cnt;

		if (this->get_res_cnt > this->res_max - 1)
			pthread_cond_broadcast(&this->put_cond);

		this->get_list = std::move(this->put_list);
		this->put_res_cnt = 0;

		pthread_mutex_unlock(&this->put_mutex);
		return this->get_res_cnt;
*/
	}

	int size()
	{
		return this->put_res_cnt + this->get_res_cnt;
	}

private:
	int res_max;

	List<T> put_list;
	List<T> get_list;

	int put_res_cnt;
	int get_res_cnt;

	pthread_mutex_t put_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t get_mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_cond_t put_cond = PTHREAD_COND_INITIALIZER;
//	struct list_head put_waiter_list;
	struct list_head get_waiter_list;
};
#endif
