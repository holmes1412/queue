#ifndef __MPMC_QUEUE__
#define __MPMC_QUEUE__

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "src/core/lib/iomgr/executor/mpmcqueue.h"

class MpmcQueue
{
public:
	MpmcQueue(int capacity)
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

#endif
