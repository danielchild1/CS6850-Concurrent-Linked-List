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

	while (workerNumber <= 100) {

		myMutexx.lock();
		workerNumber++;
		localWorkerNumber = workerNumber;
		myMutexx.unlock();

		int action = rand() % 100;
		int num = rand() % 99;

		if (action <= 10) {
			testLinkedList.insert(num);
			printf("insert: %d\n", num);
		}
		else if (action <= 20) {
			testLinkedList.remove(num);
			printf("remove: %d\n", num);
		}
		else {
			testLinkedList.find(num);
			//printf("found: %d\n", testLinkedList.find(num));
		}

		



	}

	




}



int main()
{
	srand(time(NULL));

	// thread* threads = new thread[4];

	// for (int i = 0; i < 4; i++) {
	// 	threads[i] = thread(test);
	// }
	// for (int i = 0; i < 4; i++) {
	// 	threads[i].join();
	// }
	printf("lets go!");
	linkedList<int> test(true);

	test.insert(15);
	test.insert(89);
	test.insert(1);
	printf("find 19: %d\n", test.find(19));
	printf("find 89: %d\n", test.find(89));
	test.remove(89);
	printf("find 89: %d\n", test.find(89));

	return 0;
}
