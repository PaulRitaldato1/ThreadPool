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
#include <type_traits>
#include <typeinfo>
#include <string_view>
#include "../../Global/Macros.h"

/* ThreadPool class */
class ThreadPool
{
public:
	/*  Constructors */
	ThreadPool(int8_t numThreads) 
	{
		createThreads(numThreads);
	}

	ThreadPool() 
	{
		const int8_t numThreads = (int8_t)std::thread::hardware_concurrency();
		createThreads(numThreads);
	}
	/* end constructors */

	//Destructor
	~ThreadPool()
	{
		shutdown = true;
		notifier.notify_all();
		for(size_t i = 0; i < threads.size(); ++i)
		{
			threads[i].join();
		}
	}

    //add any arg # function to queue
    template <typename Func, typename... Args >
    auto push(Func&& f, Args&&... args)
	{
		
		//get return type of the function

		//typedef decltype(f(args...)) RetType;
		using RetType = std::result_of_t<decltype(f)(Args...)>;
		std::packaged_task<RetType()> task(std::bind(f, std::forward<Args>(args)...));
		
		std::future<RetType> future = task.get_future();

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

	template <typename Func, typename Iterator>
	inline void map(Func&& f, Iterator begin, Iterator end)
	{

		static_assert(!is_iterator<Iterator>::value, "Begin argument needs to be an iterator");
		static_assert(!is_iterator<end>::value, "End argument needs to be an iterator");

		for(auto i = begin; i != end; ++i)
		{
			push(f, *i);
		}
	}
    /* utility functions will go here*/
   
    uint8_t getThreadCount(){
        return threads.size();
    }

private:

	//use SFINAE to determine types of template params
	template<typename T, typename = void>
	struct is_iterator
	{
		static constexpr bool value = true;
	};

	template<typename T>
	struct is_iterator<T, typename std::enable_if<!std::is_same<typename std::iterator_traits<T>::value_type, void>::value>::type>
	{
		static constexpr bool value = false;
	};
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

	void createThreads(uint8_t numThreads) 
	{
		auto threadFunc = [this]() 
		{
			while (true) 
			{
				std::packaged_task<void()> job;
				
				{
					std::unique_lock<std::mutex> lock(JobMutex);
					notifier.wait(lock, [this] {return !jobQueue.empty() || shutdown; });
					
					if(shutdown)
					{
						break;
					}

					// //strange bug where it will continue even if the job queue is empty
					// if (jobQueue.size() < 1)
					// 	continue;

					job = std::move(jobQueue.front());

					jobQueue.pop();
				}
				job();
				//(*job).execute();
			}
			
		};

		threads.reserve(numThreads);
		for (int i = 0; i != numThreads; ++i) 
		{
			threads.emplace_back(threadFunc);
		}
	}


	NULL_COPY_AND_ASSIGN(ThreadPool);
}; /* end ThreadPool Class */
