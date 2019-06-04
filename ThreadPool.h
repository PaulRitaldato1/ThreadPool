#pragma once

#include<thread>
#include<vector>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<future>

#define MAX_THREADS std::thread::hardware_concurrency() - 1;

//portable way to null the copy and assignment operators
#define NULL_COPY_AND_ASSIGN(T) \
	T(const T& other) {(void)other;} \
	void operator=(const T& other) { (void)other; }

/* ThreadPool class
It is a singleton, meaning there may only be one instance of this class at a time.
These are rarely used but this class uses threads on your cpu. To prevent spawning
tons of threads, I made it a singleton */
class ThreadPool{
public:

    //getInstance to allow the second constructor to be called
    static ThreadPool& getInstance(int numThreads){
        static ThreadPool instance(numThreads);

        return instance;
    }

    //add any arg # function to queue
    template <typename Func, typename... Args >
    inline auto push(Func&& f, Args&&... args){

		//get return type of the function
		typedef decltype(f(args...)) retType;

		//package the task
		std::packaged_task<retType()> task(std::move(std::bind(f, args...)));

		// lock jobqueue mutex, add job to the job queue 
		std::unique_lock<std::mutex> lock(JobMutex);
		
		//get the future from the task before the task is moved into the jobqueue
		std::future<retType> future = task.get_future();

		//place the job into the queue
		JobQueue.emplace( std::make_shared<AnyJob<retType> > (std::move(task)) );

		//notify a thread that there is a new job
        thread.notify_one();

		//return the future for the function so the user can get the return value
		return future;
    }

    /* utility functions will go here*/
    inline void resize(int newTCount){
        
        int tmp = MAX_THREADS;
        if(newTCount > tmp || newTCount < 1){
			tmp = numThreads;
            numThreads = MAX_THREADS;
			Pool.resize(newTCount);
			for (int i = tmp; i != numThreads; ++i) {
				Pool.emplace_back(std::thread(&ThreadPool::threadManager, this));
				Pool.back().detach();
			}
        }
		else if (newTCount > numThreads) {
			uint8_t tmp = numThreads;
			numThreads = newTCount;
			Pool.resize(numThreads);
			for (int i = tmp; i != numThreads; ++i) {
				Pool.emplace_back(std::thread(&ThreadPool::threadManager, this));
				Pool.back().detach();
			}
		}
		else {
			numThreads = (uint8_t)newTCount;
			Pool.resize(newTCount);
		}
        

    }

    inline uint8_t getThreadCount(){
        return numThreads;
    }

private:
	class Job {
	private:
		std::packaged_task<void()> func;
	public:
		virtual ~Job() {}
		virtual void execute() = 0;
	};

	template <typename RetType>
	class AnyJob : public Job {
	private:
		std::packaged_task<RetType()> func;
	public:
		AnyJob(std::packaged_task<RetType()> func) : func(std::move(func)) {}
		void execute() {
			func();
		}
	};

	uint8_t numThreads; // number of threads in the pool
    std::vector<std::thread> Pool; //the actual thread pool
	std::queue<std::shared_ptr<Job>> JobQueue;
    std::condition_variable thread;// used to notify threads about available jobs
	std::mutex JobMutex; // used to push/pop jobs to/from the queue

    /* infinite loop function */
	inline void threadManager() {
		while (true) {

			std::unique_lock<std::mutex> lock(JobMutex);
			thread.wait(lock, [this] {return !JobQueue.empty(); });

			//strange bug where it will continue even if the job queue is empty
			if (JobQueue.size() < 1)
				continue;

			(*JobQueue.front()).execute();

			JobQueue.pop();
		}
	}

    /*  Constructors */
    ThreadPool(); //prevent default constructor from being called

	//real constructor that is used
    inline ThreadPool(uint8_t numThreads) : numThreads(numThreads) {
        int tmp = MAX_THREADS;
        if(numThreads > tmp){
			numThreads = tmp;
		}
		Pool.reserve(numThreads);
        for(int i = 0; i != numThreads; ++i){
            Pool.emplace_back(std::thread(&ThreadPool::threadManager, this));
            Pool.back().detach();
        }
    }
    /* end constructors */


NULL_COPY_AND_ASSIGN(ThreadPool);
}; /* end ThreadPool Class */