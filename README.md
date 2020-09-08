# queue

一些消息队列的简单demo实现

## 基本分类

由于正在学习不同队列的实现，这里列举了一些简单的模型，都是多生产者多消费者使用对，基于CAS和work stealing的队列模型暂时还不在这个demo的尝试范围内。

- basic_queue: 单锁、单条件变量，可以使用锁外唤醒
- double_lock_queue: 双锁、双条件变量
- double_list_queue: workflow内部使用的双队列模型，双锁、双条件变量、双队列
- mpmc_queue: grpc内部的多生产者多消费者队列模型，单锁、若干条件变量、单队列、外加一个等待者队列（避免唤醒惊群且优化了cache line）

写了几篇鶸鶸的笔记，在知乎：

[消息队列优化 -- 鶸的介绍篇](https://zhuanlan.zhihu.com/p/110550451)

[消息队列优化 -- 几种基本实现](https://zhuanlan.zhihu.com/p/110556031)

[消息队列优化 -- grpc MPMCQueue 简介及各队列性能对比](https://zhuanlan.zhihu.com/p/196788873)

鶸鶸的我也会持续学习和改进～本demo也会持续更新～

## 一个简单的对比图
![IMG](/pictures/queue_diff.png)

## 一些简单的空跑测试结果
![IMG](/pictures/queue_table1_demo.png)
