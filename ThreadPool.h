#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <type_traits>
#include <typeinfo>

/* ThreadPool class */
class ThreadPool
{
public:
	/*  Constructors */
	ThreadPool(size_t numThreads)
	{
		createThreads(numThreads);
	}

	ThreadPool()
	{
		const size_t numThreads = (size_t)std::thread::hardware_concurrency();
		createThreads(numThreads);
	}
	/* end constructors */

	//Destructor
	~ThreadPool()
	{
		shutdown = true;
		notifier.notify_all();
		for (size_t i = 0; i < threads.size(); ++i)
		{
			threads[i].join();
		}
	}

	//add any arg # function to queue
	template <typename Func, typename... Args>
	auto enqueue(Func&& f, Args&&... args)
	{

		//typedef decltype(f(args...)) RetType;

		//get return type of the function
		using RetType = std::invoke_result_t<Func, Args...>;

		auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(f, std::forward<Args>(args)...));
		
		{
			// lock jobQueue mutex, add job to the job queue 
			std::unique_lock<std::mutex> lock(JobMutex);

			//place the job into the queue
			jobQueue.emplace([=] {
				(*task)();
			});
		}
		notifier.notify_one();

		return task->get_future();
	}
	
	/* utility functions will go here*/
	size_t getThreadCount() {
		return threads.size();
	}

private:

	using Job = std::function<void()>;
	std::vector<std::thread> threads;
	std::queue<Job> jobQueue;
	std::condition_variable notifier;
	std::mutex JobMutex;
	std::atomic<bool> shutdown = false;

	void createThreads(size_t numThreads)
	{

		threads.reserve(numThreads);
		for (int i = 0; i != numThreads; ++i)
		{
			threads.emplace_back([this]()
			{
				while (true)
				{
					Job job;

					{
						std::unique_lock<std::mutex> lock(JobMutex);
						notifier.wait(lock, [this] {return !jobQueue.empty() || shutdown; });

						if (shutdown)
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
					}
			});
		}
	}
}; /* end ThreadPool Class */
