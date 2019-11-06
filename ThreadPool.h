#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <assert.h>
#include <atomic>

//updated C++11 and on way to null copy and assign
//I like to keep it as a macro for big projects so I can just call it on any class
#define NULL_COPY_AND_ASSIGN(T) \
	T(const T& other) = delete; \
	void operator=(const T& other) = delete;

/* ThreadPool class
No longer a singleton */
class ThreadPool{
public:
	/*  Constructors */
	ThreadPool(uint8_t numThreads) {
		assert(numThreads > 0);
		createThreads(numThreads);
	}

	ThreadPool() {
		const uint8_t numThreads = (uint8_t)std::thread::hardware_concurrency();
		assert(numThreads > 0);
		createThreads(numThreads);
	}
	/* end constructors */

	//Destructor
	~ThreadPool(){
		shutdown = true;
		notifier.notify_all();
		for(int i = 0; i < threads.size(); ++i){
			threads[i].join();
		}
	}

    //add any arg # function to queue
    template <typename Func, typename... Args >
    auto push(Func&& f, Args&&... args){
		
	    assert(!std::is_bind_expression<decltype(f)>::value);

		//get return type of the function
		typedef decltype(f(args...)) retType;

		std::packaged_task<retType()> task(std::bind(f, args...));
		
		std::future<retType> future = task.get_future();

		{
			// lock jobQueue mutex, add job to the job queue 
			std::unique_lock<std::mutex> lock(JobMutex);

			//place the job into the queue
			jobQueue.emplace(std::packaged_task<void()>(std::move(task)));
			//jobQueue.emplace(std::make_shared<AnyJob<retType> >(std::move(task)));
		}
        	notifier.notify_one();

		return future;
    }

    /* utility functions will go here*/
   
    uint8_t getThreadCount(){
        return threads.size();
    }

private:

//used polymorphism to store any job type, turns out this is not needed. The type erasure that i was manually doing here, is handled by std::packaged_task<void()>
	// class Job {
	// public:
	// 	virtual void execute() = 0;
	// };

	// template <typename RetType>
	// class AnyJob : public Job {
	// private:
	// 	std::packaged_task<RetType()> func;
	// public:
	// 	AnyJob(std::packaged_task<RetType()> func) : func(std::move(func)) {}
	// 	void execute() {
	// 		func();
	// 	}
	// };

    std::vector<std::thread> threads;
	std::queue<std::packaged_task<void()>> jobQueue;
    std::condition_variable notifier;
	std::mutex JobMutex;
	std::atomic<bool> shutdown = false;

	void createThreads(uint8_t numThreads) {
		auto threadFunc = [this]() {
			while (true) {
				std::packaged_task<void()> job;
				
				{
					std::unique_lock<std::mutex> lock(JobMutex);
					thread.wait(lock, [this] {return !jobQueue.empty(); });

					if(shutdown){
						break;
					}

					//strange bug where it will continue even if the job queue is empty
					if (jobQueue.size() < 1)
						continue;

					job = std::move(jobQueue.front());

					jobQueue.pop();
				}
				job();
				//(*job).execute();
			}

		};
		threads.reserve(numThreads);
		for (int i = 0; i != numThreads; ++i) {
			threads.emplace_back(std::thread(threadFunc));
		}
	}


NULL_COPY_AND_ASSIGN(ThreadPool);
}; /* end ThreadPool Class */
