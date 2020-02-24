#include "basic_queue.h"
#include "out_list_queue.h"

typedef BasicQueue Queue;

int main()
{
	Queue queue(3);   // 使用capacity = 3初始化队列。

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
