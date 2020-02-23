#ifndef __SELF_DEFINE_LIST__
#define __SELF_DEFINE_LIST__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

template<typename E>
class Node
{
public:
	E val;
	Node *next;
 	Node(E x) : val(x), next(NULL) {}
};

// not thread safe
template<typename E>
class List
{
public:
	List()
		: head(0)
	{
		this->tail = &this->head;
	}

	List& operator=(List&& move)
	{
		if (this != &move)
		{
			this->head.next = move.head.next;
			this->tail = move.tail;
			move.tail = &move.head;
			move.head.next = NULL;
		}
		return *this;
	}

	void add_tail(E val)
	{
		Node<E> *cur = new Node<E>(val);
		this->tail->next = cur;
		this->tail = cur;
	}

	bool get_head(E& val)
	{
		if (this->tail == &this->head)
			return false;

		Node<E> *cur;
		cur = this->head.next;
		if (cur->next)
		{
			this->head.next = cur->next;
		} else {
			this->head.next = NULL;
			this->tail = &this->head;
		}

		val = cur->val;
		delete cur;

		return true;
	}

	bool empty()
	{
		return !(this->tail == &this->head);
	}

private:
	Node<E> head;
	Node<E> *tail;
};

#endif
