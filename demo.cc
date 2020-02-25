#include "basic_queue.h"
#include "double_list_queue.h"
#include "double_lock_queue.h"

typedef DoubleLockQueue Queue;

int main()
{
	Queue queue(3);   // 使用capacity = 3初始化队列。

	queue.enqueue(1);   // 生产者线程P1将1插入队列。
	printf("size=%d\n", queue.size());
	queue.enqueue(0);   // 生产者线程P2将0插入队列。
	printf("size=%d\n", queue.size());
	queue.enqueue(2);   // 生产者线程P3将2插入队列。
	printf("size=%d\n", queue.size());
	queue.dequeue();	// 消费者线程C1调用dequeue。
	printf("size=%d\n", queue.size());
	queue.dequeue();	// 消费者线程C2调用dequeue。
	printf("size=%d\n", queue.size());
	queue.dequeue();	// 消费者线程C3调用dequeue。
	printf("size=%d\n", queue.size());
	queue.enqueue(3);   // 其中一个生产者线程将3插入队列。
	printf("size=%d\n", queue.size());
	return 0;
}
