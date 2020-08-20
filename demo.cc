#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "basic_queue.h"
#include "double_list_queue.h"
#include "double_lock_queue.h"
#include "mpmc_queue.h"
#include "example.thread.h"

//typedef DoubleLockQueue Queue;
typedef DoubleListQueue<int> Queue;
//typedef BasicQueue<int> Queue;
//typedef GrpcMpmcQueue Queue;
//typedef MpmcQueue Queue;
//typedef threadsafe_queue<int> Queue;

#define PRODUCER_NTHREADS 10
#define CONSUMER_NTHREADS 10
#define QUEUE_MAX         4096000
#define BATCH_NUM         1000000

struct thread_info_t
{
    pthread_t tid;
	int threadid;
    Queue *queue;
	int begin;
	int end;
};

static void *producer_routine(void *data)
{
	struct thread_info_t *info = (struct thread_info_t *)data;
	Queue *queue = info->queue;
	int begin = info->begin;
	int end = info->end;

	for (int i = begin; i < end; i++)
	{
		queue->enqueue(i);
//		fprintf(stderr, "[%d] enqueue(%d), size = %d\n",
//				info->threadid, i, queue->size());
	}
//	fprintf(stderr, "producer-%d finish. thread-%zu size = %d\n",
//			info->threadid, info->tid, queue->size());
}

static void *consumer_routine(void *data)
{
	struct thread_info_t *info = (struct thread_info_t *)data;
	Queue *queue = info->queue;
	int begin = info->begin;
	int end = info->end;
	int ret;

	for (int i = begin; i < end; i++)
	{
		ret = queue->dequeue();
//		if (ret > BATCH_NUM)
//		fprintf(stderr, "[%d] dequeue()=%d, size = %d\n",
//				info->threadid, ret, queue->size());
	}
//	fprintf(stderr, "consumer-%d finish. thread-%zu size = %d\n",
//			info->threadid, info->tid, queue->size());
}

int main(int argc, char *argv[])
{
	int producer_nthreads = PRODUCER_NTHREADS;
	int consumer_nthreads = CONSUMER_NTHREADS;
	int queue_max = QUEUE_MAX;
	int batch_num = BATCH_NUM;
	int ret;

	if (argc == 2 || argc > 5)
	{
		fprintf(stderr, "Usage: %s [producer_nthreads] [consumer_nthreads] \
				[queue_max]\n\tDefault: producer_nthreads = %d, \
				consumer_nthreads = %d, queue_max = %d, batch_num = %d\n",
				argv[0],
				PRODUCER_NTHREADS, CONSUMER_NTHREADS, QUEUE_MAX, BATCH_NUM);
		return 0;
	}

	if (argc > 2)
	{
		producer_nthreads = atoi(argv[1]);
		consumer_nthreads = atoi(argv[2]);
	}

	if (argc > 3)
		queue_max = atoi(argv[2]);

	if (argc > 4)
		batch_num = atoi(argv[3]);

	if (producer_nthreads < 0 || consumer_nthreads < 0 ||
		queue_max < 0 || batch_num <= 0)
	{
		fprintf(stderr, "Invalid parameters.\n");
		return 0;
	}

	Queue queue(queue_max);

	struct thread_info_t *producer = NULL;
	struct thread_info_t *consumer = NULL;

	producer = (struct thread_info_t *)calloc(producer_nthreads,
											  sizeof(struct thread_info_t));
	consumer = (struct thread_info_t *)calloc(consumer_nthreads,
											  sizeof(struct thread_info_t));

	if (!producer || !consumer)
	{
		fprintf(stderr, "Calloc failed.\n");
		return 0;
	}

	for (int i = 0; i < producer_nthreads; i++)
	{
		producer[i].threadid = i;
		producer[i].queue = &queue;
		producer[i].begin = i * (batch_num / producer_nthreads);
		producer[i].end = (i + 1) * (batch_num / producer_nthreads);
		ret = pthread_create(&producer[i].tid, NULL, producer_routine,
							 &producer[i]);
		if (ret)
		{
			fprintf(stderr, "pthread_create failed.\n");
			return 0;
		}
	}

	for (int i = 0; i < consumer_nthreads; i++)
	{
		consumer[i].threadid = i;
		consumer[i].queue = &queue;
		consumer[i].begin = i * (batch_num / consumer_nthreads);
		consumer[i].end = (i + 1) * (batch_num / consumer_nthreads);
		ret = pthread_create(&consumer[i].tid, NULL, consumer_routine, &consumer[i]);
		if (ret)
		{
			fprintf(stderr, "pthread_create failed.\n");
			return 0;
		}
	}

	for (int i = 0; i < producer_nthreads; i++)
	{
		ret = pthread_join(producer[i].tid, NULL);
		if (ret)
		{
			fprintf(stderr, "pthread_jion failed.\n");
			return 0;
		}
	}

	for (int i = 0; i < consumer_nthreads; i++)
	{
		ret = pthread_join(consumer[i].tid, NULL);
		if (ret)
		{
			fprintf(stderr, "pthread_jion failed.\n");
			return 0;
		}
	}

	fprintf(stderr, "End and check queue.size() == %d\n", queue.size());

	free(consumer);
	free(producer);

	return 0;
}
