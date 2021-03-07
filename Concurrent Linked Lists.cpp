// Concurrent Linked Lists.cpp : Defines the entry point for the application.
//

#include "Concurrent Linked Lists.h"
#include <iostream>
#include <memory>
#include <cstdio>
#include <thread>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <mutex>


using std::mutex;
using std::thread;

mutex myMutexx;

unsigned int workerNumber = 0;

void test() {
	unsigned int localWorkerNumber;
	linkedList<int> testLinkedList(true);

	while (workerNumber <= 1000) {

		myMutexx.lock();
		workerNumber++;
		localWorkerNumber = workerNumber;
		myMutexx.unlock();

		int action = rand() % 100;
		int num = rand() % 99;

		if (action <= 10) {
			testLinkedList.insert(num);
		}
		else if (action <= 20) {
			testLinkedList.remove(num);
		}
		else {
			testLinkedList.find(num);
		}

		



	}

	




}



int main()
{
	srand(time(NULL));

	thread* threads = new thread[4];

	for (int i = 0; i < 4; i++) {
		threads[i] = thread(test);
	}
	for (int i = 0; i < 4; i++) {
		threads[i].join();
	}

	return 0;
}
