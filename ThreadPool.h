#include<thread>
#include<vector>
#include<queue>
#include<exception>
#include<string>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<iostream>

//portable way to null the copy and assignment operators
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

    /* Temporarily Removed until I can figure out how to have a singleton with multiple constructors*/
    //getting the instance of the class, or creating one if it does not alread exist
    // static ThreadPool& getInstance(){
    //     static ThreadPool instance;
    //     return instance;
    // }

    //overloaded getInstance to allow the second constructor to be called
    static ThreadPool& getInstance(int numThreads){
        static ThreadPool instance(numThreads);

        return instance;
    }

    //add any arg # function to queue
    template <typename Func, typename... Args >
    inline void push(Func f, Args... args){
        auto funcToAdd = std::bind(f, args...);
        {
            std::unique_lock<std::mutex> lock(JobMutex);
            JobQueue.push(funcToAdd);
        }
        thread.notify_one();
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
        
        int tmp = std::thread::hardware_concurrency() - 1;
        if(newTCount > tmp || netTCount < 1){
            throw bad_thread_alloc("Cannot allocate " + std::to_string(numThreads) + " because it is greater than your systems maximum of " + std::to_string(tmp), __FILE__, __LINE__);
        }
        
        numThreads = (uint8_t)newTCount;
        Pool.resize(newTCount);
        DEBUG("New size is: " + std::to_string(Pool.size()));
    }

    inline uint8_t getThreadCount(){
        return numThreads;
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
    ThreadPool(); //prevent default constructor from being called
    /* Temporarily Removed until I can figure out how to have a singleton with multiple constructors*/

    // inline ThreadPool(){
    //    numThreads = (uint8_t)std::thread::hardware_concurrency() - 1;
    //    if(numThreads < 0 )
    //         numThreads = 1;
    //     for (int i = 0; i != numThreads; ++i){

    //         Pool.push_back(std::thread(&ThreadPool::threadManager, this));
    //         Pool.back().detach();
    //         DEBUG("Thread " + std::to_string(i) + " allocated");
    //     }
    //     DEBUG("Number of threads being allocated " + std::to_string(numThreads));
    // }

    inline ThreadPool(uint8_t numThreads) : numThreads(numThreads) {
        int tmp = std::thread::hardware_concurrency() - 1;
        if(numThreads > tmp){
            throw bad_thread_alloc("Cannot allocate " + std::to_string(numThreads) + " because it is greater than your systems maximum of " + std::to_string(tmp), __FILE__, __LINE__);
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
    while(true){
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(JobMutex);
            thread.wait(lock, [this]{return !JobQueue.empty();});

            //strange bug where it will continue even if the job queue is empty
            if(JobQueue.size() < 1)
                continue;

            job = JobQueue.front();
            JobQueue.pop();


        }
        job();
    }
}
