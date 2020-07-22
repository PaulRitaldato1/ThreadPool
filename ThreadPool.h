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
#include <array>

/* ThreadPool class */
template <std::size_t PoolSize = 1>
class ThreadPool
{
public:
	/*  Constructors */
	//DEPRECATE TAG: Changed create threads to use compile time template param
	//ThreadPool(std::size_t numThreads)
	//END TAG
	ThreadPool()
	{
		//DEPRECATE TAG: Changed create threads to use compile time template param
		//createThreads(numThreads);
		//END TAG
		createThreads();
	}

	//DEPRECATE TAG: Removing this default constructor in favor of compile time 
		//ThreadPool()
		//{
		//	const size_t numThreads = (size_t)std::thread::hardware_concurrency();
		//	createThreads(numThreads);
		//}
	/* end constructors */
	//END TAG

	//Destructor
	~ThreadPool()
	{
		shutdown = true;
		notifier.notify_all();
		for (std::size_t i = 0; i < PoolSize; ++i)
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
	constexpr std::size_t getThreadCount() const {
		return threads.size();
	}

private:

	using Job = std::function<void()>;
	std::array<std::thread, PoolSize> threads;
	std::queue<Job> jobQueue;
	std::condition_variable notifier;
	std::mutex JobMutex;
	std::atomic<bool> shutdown = false;

	void createThreads()
	{
		constexpr std::size_t numThreads = PoolSize;
		for (int i = 0; i != numThreads; ++i)
		{
			threads[i] = std::thread([this]()
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
