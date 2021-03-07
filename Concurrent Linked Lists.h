// Concurrent Linked Lists.h : Include file for standard system include files,
// or project specific include files.


#include <memory>
#include <cstdio>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>

using std::atomic_compare_exchange_strong; 
using std::atomic;
using std::shared_ptr;
using std::shared_mutex;
using std::unique_lock;
using std::shared_lock;
using namespace std;


template <typename T>
class Node {
public:
	T value;
	shared_ptr<Node<T>> next;
	shared_ptr<Node<T>> prev;
};

template <typename T>
class AtomicNode {
public:
	T value;
	atomic<shared_ptr<AtomicNode<T>>> next;
	atomic<shared_ptr<AtomicNode<T>>> prev;
};


template <typename T>
class linkedList {
private:
	shared_ptr<Node<T>> front = std::make_shared<Node<T>>();
	shared_ptr<Node<T>> back = std::make_shared<Node<T>>();
	//atomic<shared_ptr<AtomicNode<T>>> aFront;
	//atomic<shared_ptr<AtomicNode<T>>> aBack;
	shared_mutex myMutex;
	int numNodes = 0;
	bool useLocks;


#pragma region insert
	void appendLockFree(T newNode) {

	}
	void appendLocked(T newNode) {
		shared_ptr<Node<T>> NewNode = std::make_shared<Node<T>>();
		NewNode->value = newNode;

		for (shared_ptr<Node<T>> temp = front; temp != back; temp = temp->next) {
			if (temp->next == back) {
				unique_lock lock(myMutex);
				NewNode->next = back;
				NewNode->prev = temp;
				temp->next = NewNode;
				back->prev = NewNode;
				break;
			}
			else if (newNode < temp->next->value) {
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
	void removeLockFree(T removalNode) {

	}
	void removeLocked(T removealNode) {
		for (shared_ptr<Node<T>> temp = front; temp != back; temp = temp->next) {
			if (temp->value == removealNode) {
				unique_lock lock(myMutex);
				temp->prev->next = temp->next;
				temp->next->prev = temp->prev;
				numNodes--;
				break;
			}
		}
	}
#pragma endregion
	bool findLocked(T val) {
		for (shared_ptr<Node<T>> temp = front; temp != back; temp = temp->next) {
			if (temp->value == val) {
				return true;
			}
			if (temp->value > val) {
				return false;
			}
		}
		return false;
	}

public:
	linkedList(bool locks ): useLocks(locks) {
		if (locks) {
			front->next = back;
			back->prev = front;
		}
		else {
	
		}
	}
	void insert(T newNode) {
		numNodes++;
		if (useLocks) {
			appendLocked(newNode);
		}
		else {
			appendLockFree(newNode);
		}
	}

	bool find(T value) {
		shared_lock lock(myMutex);
		if (useLocks) {
			return findLocked(value);
		}
		return false;
	}

	void remove(T removalNode) {
		if (useLocks) {
			removeLocked(removalNode);
		}
		// else {
		// 	//removeLockedFree(removalNode);
		// }
	}

};


// TODO: Reference additional headers your program requires here.
