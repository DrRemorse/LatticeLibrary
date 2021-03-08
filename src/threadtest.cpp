#include <library/sleep.hpp>
#include <iostream>

//using namespace library;

void threadTestFunc(void* data)
{
	int i = *(int*)data;
	library::sleepMillis(i * 1000);
}

void test_threads()
{
	/*
	const int THREADS = 16;
	
	// thread testing
	ThreadPool tp(THREADS);
	int waitTime = 1;
	
	while (true)
	{
		std::vector<std::future<void>> futures;
		
		std::cout << "Starting threads..." << std::endl;
		
		for (int i = 0; i < THREADS; i++)
		{
			futures.emplace_back(
				tp.startJob(threadTestFunc, (void*) &waitTime)
			);
		}
		
		std::cout << "Waiting for threads..." << std::endl;
		
		tp.finishQueue(futures);
		
		std::cout << "Finished waiting for threads!" << std::endl;
	}
	*/
	std::getchar();
}
