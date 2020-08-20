#ifndef __BASIC_QUEUE__
#define __BASIC_QUEUE__

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"

template<typename T>
class BasicQueue
{
public:
	BasicQueue(int capacity)
	{
		this->res_max = capacity;
		this->res_cnt = 0;
		this->put_wait_cnt = 0;
	}

	void enqueue(T element)
	{
		pthread_mutex_lock(&this->mutex);
		while (this->res_cnt >= this->res_max)
		{
			this->put_wait_cnt++;
			pthread_cond_wait(&this->put_cond, &this->mutex);
			this->put_wait_cnt--;
		}

		this->res_list.add_tail(element);
		this->res_cnt++;

//		if (this->res_cnt == 1)
		pthread_mutex_unlock(&this->mutex);
		pthread_cond_signal(&this->get_cond);
	}

	T dequeue()
	{
		int ret;

		pthread_mutex_lock(&this->mutex);
		while (this->res_cnt == 0)// && this->res_list.empty())
			pthread_cond_wait(&this->get_cond, &this->mutex);

		if (this->res_cnt != 0)
		{
			this->res_list.get_head(ret);
			this->res_cnt--;
//			if (this->put_wait_cnt > 0)
//				pthread_cond_signal(&this->put_cond);
		}

		pthread_mutex_unlock(&this->mutex);
		pthread_cond_signal(&this->put_cond);
		return ret;
	}

	int size()
	{
		return this->res_cnt;
	}

private:
	int res_max;
	int res_cnt;
	int put_wait_cnt;

	List<T> res_list;

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t put_cond = PTHREAD_COND_INITIALIZER;
	pthread_cond_t get_cond = PTHREAD_COND_INITIALIZER;
};

#endif
