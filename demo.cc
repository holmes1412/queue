#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "basic_queue.h"
#include "double_list_queue.h"
#include "double_lock_queue.h"

typedef DoubleLockQueue Queue;

struct thread_info_t
{
    pthread_t tid;
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
		fprintf(stderr, "thread-%d enqueue(%d), size = %d\n",
				info->tid, i, queue->size());
	}
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
		fprintf(stderr, "thread-%d dequeue()=%d, size = %d\n",
				info->tid, ret, queue->size());
	}
}
int main(int argc, char *argv[])
{
	int producer_nthreads = 10;
	int consumer_nthreads = 10;
	int queue_max = 6;
	int batch = 1000;
	int ret;

	if (argc == 2 || argc > 4)
	{
		fprintf(stderr, "Usage: %s [producer_nthreads] [consumer_nthreads] \
				[queue_max]\n\tDefault: producer_nthreads = 10, \
				consumer_nthreads = 10, queue_max = 1024\n", argv[0]);
		return 0;
	}

	if (argc > 2)
	{
		producer_nthreads = atoi(argv[1]);
		consumer_nthreads = atoi(argv[2]);
	}

	if (argc > 3)
		queue_max = atoi(argv[2]);

	if (producer_nthreads < 0 || consumer_nthreads < 0 || queue_max < 0)
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
		producer[i].queue = &queue;
		producer[i].begin = i * (batch / producer_nthreads);
		producer[i].end = (i + 1) * (batch / producer_nthreads);
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
		consumer[i].queue = &queue;
		consumer[i].begin = i * (batch / consumer_nthreads);
		consumer[i].end = (i + 1) * (batch / consumer_nthreads);
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

	//check
	fprintf(stderr, "End and queue.size() == %d\n", queue.size());

	free(consumer);
	free(producer);

	return 0;
}
