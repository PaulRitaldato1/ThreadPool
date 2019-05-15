#include<thread>
#include<vector>
#include<queue>
#include<exception>
#include<string>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<iostream>
#include<chrono>

#define NULL_COPY_AND_ASSIGN(T) \
	T(const T& other) {(void)other;} \
	void operator=(const T& other) { (void)other; }

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
   //const char* what() const throw() {
   //     std::string what = "bad_thread_alloc: In file " + file + ": " + info + " on line " + std::to_string(line);
   //     return what.c_str();
   // }

public:
    bad_thread_alloc(const std::string info, const char* file, int line) 
    {
        std::string tmp(file);
        this->file;
        this->info = tmp;
        this->line = line;
    }
}; 


/* ThreadPool class
It is a singleton, meaning there may only be one instance of this class at a time.
These are rarely used but this class uses threads on your cpu. To prevent spawning
tons of threads, I made it a singleton */
class ThreadPool{
public:

    //getting the instance of the class, or creating one if it does not alread exist
    static ThreadPool& getInstance(){
        static ThreadPool instance;

        return instance;
    }

    //overloaded getInstance to allow the second constructor to be called
    static ThreadPool& getInstance(int numThreads){
        static ThreadPool instance(numThreads);

        return instance;
    }


    /* overloaded push_back methods */ 

    //add no arg function to queue
    void push_back(std::function<void()> job);

    // //add 1 arg function to queue
     template <typename arg>
     void push_back(std::function<void(arg)> job, arg arg1);

    //add 2 arg function to queue
    template <typename argu1, typename argu2>
    void push_back(std::function<void(argu1, argu2)>job, argu1 arg1, argu2 arg2);

    //add 3 arg function to queue
    template <typename a1, typename a2, typename a3>
    void push_back(std::function<void(a1, a2, a3)> job, a1 arg1, a2 arg2, a3 arg3);

    //add 4 arg function to queue
    template <typename ar1, typename ar2, typename ar3, typename ar4>
    void push_back(std::function<void(ar1, ar2, ar3, ar4)> job, ar1 arg1, ar2 arg2, ar3 arg3, ar4 arg4);

    /* map function (similar to pythons map function) */
    template <typename argument>
    inline void map(std::function<void(argument)> function, std::vector<argument> iterable){
        for(auto arg : iterable){
            push_back(function, arg);
        }
    }
private:

    uint8_t numThreads;
    std::vector<std::thread> Pool;
    std::queue<std::function<void()>> JobQueue;
    std::condition_variable thread;
    std::mutex JobMutex;
    /* infinite loop function */
    void threadManager();

    /*  Constructors */
    inline ThreadPool(){
       numThreads = (uint8_t)std::thread::hardware_concurrency() - 1;
       if(numThreads < 0 )
            numThreads = 1;
        for (int i = 0; i != numThreads; ++i){
            DEBUG("Thread " + std::to_string(i) + " allocated");
            Pool.push_back(std::thread(&ThreadPool::threadManager, this));
            Pool.back().detach();
        }
        DEBUG("Number of threads being allocated " + std::to_string(numThreads));
    }
    inline ThreadPool(uint8_t numThreads) : numThreads(numThreads) {
        int tmp = std::thread::hardware_concurrency();
        if(numThreads > tmp){
            throw bad_thread_alloc("Cannot allocate " + std::to_string(numThreads) + " because it is greater than your systems maximum of " + std::to_string(tmp), __FILE__, __LINE__);
        }
        for(int i = 0; i != numThreads; ++i){
            DEBUG("Thread " + std::to_string(i) + " allocated");
            Pool.push_back(std::thread(&ThreadPool::threadManager, this));
            Pool.back().detach();
        }
        DEBUG("Number of threads being allocated " + std::to_string(numThreads));
    }
    /* end constructors */


NULL_COPY_AND_ASSIGN(ThreadPool);
}; /* end ThreadPool Class */

void ThreadPool::threadManager(){
    while(true){
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(JobMutex);
            thread.wait(lock, [this]{return !JobQueue.empty();});
            if(JobQueue.size() < 1)
                continue;

            job = JobQueue.front();
            JobQueue.pop();


        }
        job();
    }
}

void ThreadPool::push_back(std::function<void()> job){
    {
        std::unique_lock<std::mutex> lock(JobMutex);
        JobQueue.push(job);
    }
    thread.notify_one();
}


 //add 1 arg function to queue
template <typename arg>
void ThreadPool::push_back(std::function<void(arg)> job, arg arg1){
	auto tmpJob = std::bind(job, arg1);
//     DEBUG("made it here");
    {
        std::unique_lock<std::mutex> lock(JobMutex);
        JobQueue.push(tmpJob);
    }
    thread.notify_one();
}

//add 2 arg function to queue
template <typename argu1, typename argu2>
void ThreadPool::push_back(std::function<void(argu1, argu2)> job, argu1 arg1, argu2 arg2){
    auto tmpJob = std::bind(job, arg1, arg2);
    {
        std::unique_lock<std::mutex> lock(JobMutex);
        JobQueue.push(tmpJob);
    }
    thread.notify_one();
}

//add 3 arg function to queue
template <typename a1, typename a2, typename a3>
void ThreadPool::push_back(std::function<void(a1, a2, a3)> job, a1 arg1, a2 arg2, a3 arg3){
    auto tmpJob = std::bind(job, arg1, arg2, arg3);
    {
        std::unique_lock<std::mutex> lock(JobMutex);
        JobQueue.push(tmpJob);
    }
    thread.notify_one();
}

//add 4 arg function to queue
template <typename ar1, typename ar2, typename ar3, typename ar4>
void ThreadPool::push_back(std::function<void(ar1, ar2, ar3, ar4)> job, ar1 arg1, ar2 arg2, ar3 arg3, ar4 arg4){
    auto tmpJob = std::bind(job, arg1, arg2, arg3, arg4);
    {
        std::unique_lock<std::mutex> lock(JobMutex);
        JobQueue.push(tmpJob);
    }
    thread.notify_one();
}
