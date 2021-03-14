// Concurrent Linked Lists.cpp : Defines the entry point for the application.
//

#include "Concurrent Linked Lists.h"
#include <memory>
#include <cstdio>
#include <thread>
#include <iostream>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <mutex>


using std::mutex;
using std::thread;

mutex myMutexx;

unsigned int workerNumber = 0;
atomic<int> aworkerNumber {0};
unsigned int numThreads;
const unsigned int numTimesToRun = 100000; 
linkedList<int> locked(true);
linkedList<int> unlocked(false);



void lockedListTest() {
	unsigned int localWorkerNumber;

	while (workerNumber <= numTimesToRun) {

		myMutexx.lock();
		localWorkerNumber = workerNumber++;
		myMutexx.unlock();

		if(localWorkerNumber > numTimesToRun){
			return;
		}

		int action = rand() % 99 + 1;
		int num = rand() % 100 +1;

		if (action <= 10) {
			locked.insert(num);
			if((localWorkerNumber % 10000) == 0){
				printf("insert: %d\n", num);
			}
			
		}
		else if (action <= 20) {
			locked.remove(num);
			if((localWorkerNumber % 10000) == 0){
				printf("remove: %d\n", num);
			}
		}
		else {
			locked.find(num);
			if((localWorkerNumber % 10000) == 0){
				printf("find: %d\n", num);
			}
		}

	}

}

void unlockedListTest() {
	int localWorkerNumber;

	while (aworkerNumber <= numTimesToRun) {

		
		localWorkerNumber = aworkerNumber;
		atomic_compare_exchange_strong( &aworkerNumber, &localWorkerNumber  , aworkerNumber+1 );

		if(localWorkerNumber > numTimesToRun){
			break;
		}

		int action = rand() % 99 + 1;
		int num = rand() % 100;

		if (action <= 10) {
			unlocked.insert(num);
			if((localWorkerNumber % 10000) == 0){
				printf("insert: %d\n", num);
			}
		}
		else if (action <= 20) {
			unlocked.remove(num);
			if(localWorkerNumber % 10000 == 0){
				printf("remove: %d\n", num);
			}
		}
		else {
			unlocked.find(num);
			if(localWorkerNumber % 10000 == 0){
				printf("find: %d\n", num);
			}
		}

	}

}


void lockedThreads(){
	thread* threads = new thread[numThreads];

	for (int i = 0; i < numThreads; i++) {
		threads[i] = thread(lockedListTest);
	}
	for (int i = 0; i < numThreads; i++) {
		threads[i].join();
	}
	
	delete[] threads;
}

void unlockedThreads(){
	thread* threads = new thread[numThreads];

	for (int i = 0; i < numThreads; i++) {
		threads[i] = thread(unlockedListTest);
	}
	for (int i = 0; i < numThreads; i++) {
		threads[i].join();
	}

	delete[] threads;
}



int main()
{

	srand(time(NULL));

	printf("How many threads: ");
	std::cin >> numThreads;

	char locked;
	printf("Locked (l) or Unlocked (u): ");
	std::cin >> locked;


	auto start = std::chrono::high_resolution_clock::now();

	if(locked == 'l'){
		start = std::chrono::high_resolution_clock::now();
		lockedThreads();
	}
	else{
		start = std::chrono::high_resolution_clock::now();
		unlockedThreads();
	}
	
	auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> running_time = end - start;

	printf("Time: %f \n", running_time.count());

	return 0;
}
