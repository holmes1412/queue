#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <atomic>

class DoubleLockQueue
{
public:
	DoubleLockQueue(int capacity)
	{
		// what if capacity == 0?
		this->res_max = capacity;
		this->res_cnt = 0;
		this->queue_cnt = 0;
	}

	void enqueue(int element)
	{
		int cnt;
		pthread_mutex_lock(&this->put_mutex);
		while (this->res_cnt >= this->res_max)
			pthread_cond_wait(&this->put_cond, &this->put_mutex);

		this->list.add_tail(element);

		cnt = __sync_fetch_and_add(&this->res_cnt, 1);

		// i can still put one more
		// can i signal outside the put_mutex?
		// should have try !!!
		if (cnt + 1 < this->res_max)
			pthread_cond_signal(&this->put_cond);

		pthread_mutex_unlock(&this->put_mutex);

		// before i add this, it`s empty and some one may be waiting for get_cond
		// signalNotEmpty();
		if (cnt == 0)
		{
			// why i must lock get_mutex
			pthread_mutex_lock(&this->get_mutex);
			pthread_cond_signal(&this->get_cond);
			pthread_mutex_unlock(&this->get_mutex);
		}
		__sync_add_and_fetch(&this->queue_cnt, 1);
//		printf("enqueue(%d) and size=%d res_cnt=%d\n",
//				element, this->size(), this->res_cnt);
	}

	int dequeue()
	{
		int ret;
		int cnt;

		pthread_mutex_lock(&this->get_mutex);
		while (this->res_cnt == 0)// && this->list.empty())
			pthread_cond_wait(&this->get_cond, &this->get_mutex);

		// don`t know whether this is necessary
		if (this->res_cnt != 0)
		{
			this->list.get_head(ret);
			cnt = __sync_fetch_and_sub(&this->res_cnt, 1);

			// something still left inside the queue
			if (cnt > 1)
				pthread_cond_signal(&this->get_cond);

//		} else {
			// this should not happen depend in this leetcode
		}

		pthread_mutex_unlock(&this->get_mutex);
		// just now i took and make one extra space
		if (cnt == this->res_max)
		{
			pthread_mutex_lock(&this->put_mutex);
			pthread_cond_signal(&this->put_cond);
			pthread_mutex_unlock(&this->put_mutex);
		}
		__sync_sub_and_fetch(&this->queue_cnt, 1);
//		printf("dequeue()=%d and size=%d res_cnt=%d\n",
//				ret, this->size(), this->res_cnt);
		return ret;
	}


	int size()
	{
		return this->queue_cnt;
	}

private:
	int res_max;
	int res_cnt;
	int queue_cnt;

	List<int> list;

	pthread_mutex_t put_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t get_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t put_cond = PTHREAD_COND_INITIALIZER;
	pthread_cond_t get_cond = PTHREAD_COND_INITIALIZER;
	// get_cond means not empty, put_cond means not full
};
