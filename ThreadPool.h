#pragma once

#include<thread>
#include<vector>
#include<queue>
#include<exception>
#include<string>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<iostream>
#include<map>
#include<memory>
#include<type_traits>
#include<future>
#include<utility>

template <typename Func, typename... Args >
void push_test(Func&& f, Args&&... args) {

	//for some reason this doesnt give me the return type
	//typedef typename std::decay<Func>::type funcType;
	//typedef typename std::invoke_result<funcType(Args...)> retTypeTest;

	//this does though
	typedef decltype(f(args...)) retType;

	std::packaged_task<retType()> task(std::move(std::bind(f, args...)));

	if (!std::is_same<retType, void>::value) {
		auto future = task.get_future();
		std::cout << typeid(future).name() << std::endl;
		std::cout << "Not void!" << std::endl;
	}
	else {
		std::cout << "Function returns void" << std::endl;
	}



	//retTypeTest i;
	//std::cout << typeid(i).name() << std::endl;



}


//portable way to null the copy and assignment operators
#define NULL_COPY_AND_ASSIGN(T) \
	T(const T& other) {(void)other;} \
	void operator=(const T& other) { (void)other; }

#define MAX_THREADS std::thread::hardware_concurrency() - 1;
//#define DBG
#ifdef DBG
#define DEBUG(x) std::cout << x << std::endl;
#else
#define DEBUG(x)
#endif

//custom exception for handling out of bounds thread creation or any bad allocation of threads
struct bad_thread_alloc : public std::exception {

private:
     std::string info;
     std::string file;
    int line;


public:
   const char* what() const throw() {
        std::string what = "bad_thread_alloc: " + info + " on line " + std::to_string(line) + " in file " + file;
        return what.c_str();
    }

    bad_thread_alloc(const std::string info, char* file, int line)
    {
        //std::string tmp(file);
        //this->file = tmp;
        this->info = info;
        this->line = line;
    }
}; 



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
            throw bad_thread_alloc("Cannot allocate " + std::to_string(newTCount) + " threads because it is greater than your systems maximum of " + std::to_string(tmp), __FILE__, __LINE__);
        }
        
        numThreads = (uint8_t)newTCount;
        Pool.resize(newTCount);
        DEBUG("New size is: " + std::to_string(Pool.size()));
    }

    inline uint8_t getThreadCount(){
        return numThreads;
    }
	
	auto getReturnValue(uint64_t jobID) {
		//Not sure how to handle this
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
    void threadManager();

    /*  Constructors */
    ThreadPool(); //prevent default constructor from being called

	//real constructor that is used
    inline ThreadPool(uint8_t numThreads) : numThreads(numThreads) {
        int tmp = MAX_THREADS;
        if(numThreads > tmp){
            throw bad_thread_alloc("Cannot allocate " + std::to_string(numThreads) + " threads because it is greater than your systems maximum of " + std::to_string(tmp), __FILE__, __LINE__);
        }
        for(int i = 0; i != numThreads; ++i){
            Pool.push_back(std::thread(&ThreadPool::threadManager, this));
            Pool.back().detach();
            DEBUG("Thread " + std::to_string(i) + " allocated");
        }
        DEBUG("Number of threads being allocated " + std::to_string(numThreads));
    }
    /* end constructors */


NULL_COPY_AND_ASSIGN(ThreadPool);
}; /* end ThreadPool Class */


void ThreadPool::threadManager(){

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