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


class task {
public:
	virtual ~task() {}
	int func;
};
template <typename T>
class AnyTask : virtual public task {
public:
	AnyTask(T func) : func(std::move(func)) {}
	T func;
};


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
    inline uint64_t push(Func& f, Args&&... args){
        auto funcToAdd = std::bind(f, args...);
		

        
		uint64_t newID = currentID++;
		std::unique_lock<std::mutex> lock(JobMutex);
		//typename std::invoke_result<Func, Args...>::type tempType;
        JobQueue.push(std::make_pair(funcToAdd, newID));
        thread.notify_one();
		return newID; //return the ID of the job in the queue
    }

    /* map function (similar to pythons map function) */
    template <typename argument>
    inline void map(std::function<void(argument)> function, std::vector<argument> iterable){
        for(auto arg : iterable){
            push(function, arg);
        }
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

	class future {};
	template <class T>
	class futureOf : public future {};
	std::map<uint64_t, std::shared_ptr<future>> returnMap;

	uint64_t currentID;
    uint8_t numThreads;
    std::vector<std::thread> Pool; //the actual thread pool
    std::queue<std::pair<std::function<void()>, uint64_t>> JobQueue; //the jobs with their assigned ID
    std::condition_variable thread;
    std::mutex JobMutex;

    /* infinite loop function */
    void threadManager();

    /*  Constructors */
    ThreadPool(); //prevent default constructor from being called

	//real constructor that is used
    inline ThreadPool(uint8_t numThreads) : numThreads(numThreads) {
		currentID = 0; //initialize currentID
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

		auto job = JobQueue.front().first;
		JobQueue.pop();
		job();
    }
}
