// Concurrent Linked Lists.h : Include file for standard system include files,
// or project specific include files.

#include <memory>
#include <cstdio>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>

using std::atomic;
using std::atomic_compare_exchange_strong;
using std::shared_lock;
using std::shared_mutex;
using std::shared_ptr;
using std::unique_lock;
using namespace std;

template <typename T>
class Node
{
public:
	T value;
	shared_ptr<Node<T>> next;
	shared_ptr<Node<T>> prev;
};

template <typename T>
class singleNode
{
public:
	T value;
	singleNode<T> *next;
};

template <typename T>
class linkedList
{
private:
	shared_ptr<Node<T>> front = std::make_shared<Node<T>>();
	shared_ptr<Node<T>> back = std::make_shared<Node<T>>();
	// atomic<shared_ptr<AtomicNode<T>>> aBack;
	// atomic<shared_ptr<AtomicNode<T>>> aFront;
	singleNode<T> *singleFront = new singleNode<T>();
	singleNode<T> *singleBack = new singleNode<T>();
	shared_mutex myMutex;
	int numNodes = 0;
	bool useLocks;

#pragma region insert
	void appendLockFree(T key)
	{
		singleNode *new_node = new Node(key);
		singleNode *right_node, *left_node;
		do
		{
			right_node = search(key, &left_node);
			if ((right_node != tail) && (right_node.key == key)) /*T1*/
				return false;
			new_node.next = right_node;
			if (atomic_compare_exchange_strong(&left_node.next, right_node, new_node)) /*C2*/
				return true;
		} while (true); /*B3*/
	}
	void appendLocked(T newNode)
	{
		shared_ptr<Node<T>> NewNode = std::make_shared<Node<T>>();
		NewNode->value = newNode;

		for (shared_ptr<Node<T>> temp = front; temp != back; temp = temp->next)
		{
			if (temp->next == back)
			{
				unique_lock lock(myMutex);
				NewNode->next = back;
				NewNode->prev = temp;
				temp->next = NewNode;
				back->prev = NewNode;
				break;
			}
			else if (newNode < temp->next->value)
			{
				unique_lock lock(myMutex);
				NewNode->next = temp->next;
				NewNode->prev = temp;
				temp->next->prev = NewNode;
				temp->next = NewNode;
				break;
			}
		}
	}
#pragma endregion

#pragma region remove

	void removeLockFree(T search_key)
	{
		singleNode *right_node, *right_node_next, *left_node;
		do
		{
			right_node = search(search_key, &left_node);
			if ((right_node == tail) || (right_node.key != search_key)) /*T1*/
				return false;
			right_node_next = right_node.next;
			if (!is_marked_reference(right_node_next))
				if (atomic_compare_exchange_strong(&(right_node.next), /*C3*/ right_node_next, get_marked_reference(right_node_next)))
					break;
		} while (true);																		 /*B4*/
		if (!atomic_compare_exchange_strong(&(left_node.next), right_node, right_node_next)) /*C4*/
			right_node = search(right_node.key, &left_node);
		return true;
	}
	void removeLocked(T removealNode)
	{
		for (shared_ptr<Node<T>> temp = front; temp != back; temp = temp->next)
		{
			if (temp->value == removealNode)
			{
				unique_lock lock(myMutex);
				temp->prev->next = temp->next;
				temp->next->prev = temp->prev;
				numNodes--;
				break;
			}
		}
	}
#pragma endregion

#pragma region search
	bool findLockFree(T search_key)
	{
		singleNode *right_node, *left_node;
		right_node = search(search_key, &left_node);
		if ((right_node == tail) || (right_node.key != search_key))
			return false;
		elsereturn true;
	}
	bool findLocked(T val)
	{
		for (shared_ptr<Node<T>> temp = front; temp != back; temp = temp->next)
		{
			if (temp->value == val)
			{
				return true;
			}
			if (temp->value > val)
			{
				return false;
			}
		}
		return false;
	}

	singleNode<T> *search(T search_key, singleNode<T> **left_node)
	{
		Node *left_node_next, *right_node;
	search_again:
		do
		{
			Node *t = head;
			Node *t_next = head.next; /* 1: Find left_node and right_node */
			do
			{
				if (!is_marked_reference(t_next))
				{
					(*left_node) = t;
					left_node_next = t_next;
				}
				t = get_unmarked_reference(t_next);
				if (t == tail)
					break;
				t_next = t.next;
			} while (is_marked_reference(t_next) || (t.key < search_key)); /*B1*/
			right_node = t;												   /* 2: Check nodes are adjacent */
			if (left_node_next == right_node)
				if ((right_node != tail) && is_marked_reference(right_node.next))
					goto search_again; /*G1*/
				else
				{
					return right_node;															/*R1*/
				}																				/* 3: Remove one or more marked nodes */
			if (atomic_compare_exchange_strong((&(left_node.next), left_node_next, right_node)) /*C1*/
				if ((right_node != tail) && is_marked_reference(right_node.next))
					goto search_again; /*G2*/
			else {
				return right_node;}	   /*R2*/
		} while (true);					   /*B2*/
	}

#pragma endregion

public:

	linkedList(bool locks) : useLocks(locks)
	{
		if (locks)
		{
			front->next = back;
			back->prev = front;
		}
		else
		{
			singleFront->next = back;
		}
	}
	void insert(T newNode)
	{
		numNodes++;
		if (useLocks)
		{
			appendLocked(newNode);
		}
		else
		{
			appendLockFree(newNode);
		}
	}

	bool find(T value)
	{
		shared_lock lock(myMutex);
		if (useLocks)
		{
			return findLocked(value);
		}
		return false;
	}

	void remove(T removalNode)
	{
		if (useLocks)
		{
			removeLocked(removalNode);
		}
		// else {
		// 	//removeLockedFree(removalNode);
		// }
	}
};

// TODO: Reference additional headers your program requires here.
