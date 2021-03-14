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
	T key;
	atomic<singleNode<T>*> next;

	singleNode(T k):key(k){};
	singleNode(){};

	singleNode<T> operator=(singleNode<T> other){
		std::swap(key, other.key);
		std::swap(next, other.next);
		return *this;
	}
};

template <typename T>
class linkedList
{
private:
	shared_ptr<Node<T>> front = std::make_shared<Node<T>>();
	shared_ptr<Node<T>> back = std::make_shared<Node<T>>();
	singleNode<T> *singleFront = new singleNode<T>();
	singleNode<T> *singleBack = new singleNode<T>();
	shared_mutex myMutex;
	int numNodes = 0;
	bool useLocks;

	bool is_marked_reference(singleNode<T>* ptr)
	{
		uint64_t val = reinterpret_cast<std::uintptr_t>(ptr);
		return (val % 2 == 1);
	}

	singleNode<T> *get_unmarked_reference(singleNode<T> *ptr)
	{
		uint64_t val = reinterpret_cast<std::uintptr_t>(ptr);
		if (val % 2 == 1)
		{
			val -= 1;
		}
		return reinterpret_cast<singleNode<T> *>(val);
	}

	singleNode<T> *get_marked_reference(singleNode<T> *ptr)
	{
		uint64_t val = reinterpret_cast<std::uintptr_t>(ptr);
		if (val % 2 == 1)
		{
			val += 1;
		}
		return reinterpret_cast<singleNode<T> *>(val);
	}

	singleNode<T>* search(T SearchKey, singleNode<T> **leftNode)
	{
		singleNode<T> *left_node_next;
		singleNode<T> *right_node;
	search_again:
		do
		{
			singleNode<T> *t = singleFront;
			singleNode<T> *t_next = singleFront->next; /* 1: Find left_node and right_node */
			do
			{
				if (!is_marked_reference(t_next))
				{
					(*leftNode) = t;
					left_node_next = t_next;
				}
				t = get_unmarked_reference(t_next);
				if (t == singleBack)
					break;
				t_next = t->next;
			} while (is_marked_reference(t_next) || (t->key < SearchKey)); /*B1*/
			right_node = t;												   /* 2: Check nodes are adjacent */
			if (left_node_next == right_node)
				if ((right_node != singleBack) && is_marked_reference(right_node->next))
					//goto search_again; /*G1*/
					continue;
				else
				{
					return right_node;
				} /*R1*/																	   /* 3: Remove one or more marked nodes */
			if (atomic_compare_exchange_strong(&((*leftNode)->next), &left_node_next, right_node)) /*C1*/
				if ((right_node != singleBack) && is_marked_reference(right_node->next))
					//goto search_again; /*G2*/
					continue;
				else
				{
					return right_node;
				}		/*R2*/
		} while (true); /*B2*/
	}

#pragma region insert
	bool appendLockFree(T key)
	{
		singleNode<T> *new_node = new singleNode(key);
		singleNode<T> *right_node;
		singleNode<T> *left_node;

		do
		{
			right_node = search(key, &left_node);
			// if ((right_node != singleBack) && (right_node->key == key)) /*T1*/
			// 	return false;
			new_node->next = right_node;
			if (atomic_compare_exchange_strong(&(left_node->next), &right_node, new_node)) /*C2*/
				return true;
		} while (true); /*B3*/
	}
	bool appendLocked(T newNode)
	{
		shared_ptr<Node<T>> NewNode = std::make_shared<Node<T>>();
		NewNode->value = newNode;

		unique_lock<shared_mutex> lock(myMutex);
		for (shared_ptr<Node<T>> temp = front; temp != back; temp = temp->next)
		{
			if (temp->next == back ||newNode < temp->next->value)
			{
				
				NewNode->next = temp->next;
				NewNode->prev = temp;
				temp->next->prev = NewNode;
				temp->next = NewNode;
				return true;
			}
		}
		return false;
	}
#pragma endregion

#pragma region remove

	bool removeLockFree(T search_key)
	{
		singleNode<T> *right_node, *right_node_next, *left_node;
		do
		{
			right_node = search(search_key, &left_node);
			if ((right_node == singleBack) || (right_node->key != search_key)) /*T1*/
				return false;
			right_node_next = right_node->next;
			if (!is_marked_reference(right_node_next))
				if (atomic_compare_exchange_strong(&(right_node->next), &right_node_next, get_marked_reference(right_node_next)))
					break;
		} while (true);																		 /*B4*/
		if (!atomic_compare_exchange_strong(&(left_node->next), &right_node, right_node_next)) /*C4*/
			right_node = search(right_node->key, &left_node);
		return true;
	}

	void removeLocked(T removealNode)
	{
		unique_lock<shared_mutex> lock(myMutex);
		for (shared_ptr<Node<T>> temp = front; temp != back; temp = temp->next)
		{
			if (temp->value == removealNode)
			{
				
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
		singleNode<T> *right_node, *left_node;
		right_node = search(search_key, &left_node);
		if ((right_node == singleBack) || (right_node->key != search_key))
			return false;
		else
		{
			return true;
		}
	}
	bool findLocked(T val)
	{
		shared_lock<shared_mutex> lock(myMutex);
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
			singleFront->next = singleBack;
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
		if (useLocks)
		{
			return findLocked(value);
		}
		else{
			return findLockFree(value);
		}
		return false;
	}

	void remove(T removalNode)
	{
		if (useLocks)
		{
			removeLocked(removalNode);
		}
		else {
			removeLockFree(removalNode);
		}
	}
};

// TODO: Reference additional headers your program requires here.
