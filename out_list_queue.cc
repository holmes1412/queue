#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#include "list.h"

class OutListQueue
{
public:
	OutListQueue(int capacity)
	{
		this->res_max = capacity;
		this->put_res_cnt = 0;
		this->get_res_cnt = 0;
	}

	void enqueue(int element)
	{

		pthread_mutex_lock(&this->put_mutex);
		while (this->put_res_cnt >= this->res_max)
			pthread_cond_wait(&this->put_cond, &this->put_mutex);

		this->put_list.add_tail(element);

		this->put_res_cnt++;

		pthread_mutex_unlock(&this->put_mutex);
		pthread_cond_signal(&this->get_cond);
	}

	int dequeue()
	{
		int ret;

		pthread_mutex_lock(&this->get_mutex);

		if (this->get_list.empty() || this->swap_list() > 0)
		{
			
			this->get_list.get_head(ret);
			this->get_res_cnt--;
//		} else {
		}

		pthread_mutex_unlock(&this->get_mutex);
		return ret;
	}


	int swap_list()
	{
		int ret;
		pthread_mutex_lock(&this->put_mutex);

		while (this->put_res_cnt == 0)
			pthread_cond_wait(&this->get_cond, &this->put_mutex);

		this->get_res_cnt = this->put_res_cnt;

		if (this->get_res_cnt > this->res_max - 1)
			pthread_cond_broadcast(&this->put_cond);

		this->get_list = std::move(this->put_list);

		this->put_res_cnt = 0;

		pthread_mutex_unlock(&this->put_mutex);

		return this->get_res_cnt;
	}

	int size()
	{
		return this->put_res_cnt + this->get_res_cnt;
	}

private:
	int res_max;

	List<int> put_list;
	List<int> get_list;

	int put_res_cnt;
	int get_res_cnt;

	pthread_mutex_t put_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t get_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t put_cond = PTHREAD_COND_INITIALIZER;
	pthread_cond_t get_cond = PTHREAD_COND_INITIALIZER;
};

int main()
{
	OutListQueue queue(3);   // 使用capacity = 3初始化队列。

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
