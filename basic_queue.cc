#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"

class BasicQueue
{
public:
	BasicQueue(int capacity)
	{
		this->res_max = capacity;
		this->res_cnt = 0;
		this->put_wait_cnt = 0;
	}

	void enqueue(int element)
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

		pthread_mutex_unlock(&this->mutex);
		pthread_cond_signal(&this->get_cond);
	}

	int dequeue()
	{
		int ret;

		pthread_mutex_lock(&this->mutex);
		while (this->res_cnt == 0 && this->res_list.empty())
			pthread_cond_wait(&this->get_cond, &this->mutex);

		if (this->res_cnt != 0)
		{
			this->res_list.get_head(ret);
			this->res_cnt--;

			if (this->put_wait_cnt > 0)
				pthread_cond_signal(&this->put_cond);
//	  } else {
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
	int put_wait_cnt;

	List<int> res_list;

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t put_cond = PTHREAD_COND_INITIALIZER;
	pthread_cond_t get_cond = PTHREAD_COND_INITIALIZER;
};

int main()
{
	BasicQueue queue(3);   // 使用capacity = 3初始化队列。

	queue.enqueue(1);   // 生产者线程P1将1插入队列。
	queue.enqueue(0);   // 生产者线程P2将0插入队列。
	queue.enqueue(2);   // 生产者线程P3将2插入队列。
	queue.dequeue();	// 消费者线程C1调用dequeue。
	queue.dequeue();	// 消费者线程C2调用dequeue。
	queue.dequeue();	// 消费者线程C3调用dequeue。
	queue.enqueue(3);   // 其中一个生产者线程将3插入队列。
	queue.size(); 
	return 0;
}
