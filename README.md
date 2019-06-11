# ThreadPool
I needed an easy to use thread pool for C++ for my game engine project, so here it goes.

All you need to do is include the header file and done!
## Usage
The ThreadPool class is a Singleton. So here is an example of how to use it.


```c++
#include "ThreadPool.h"

int main () {
  
  /* ================== Example Functions ================== */
  auto job = [](){ std::cout << "inside job" << std::endl;};
  auto jobWithArgs = [](int i){std::cout << "inside jobWithArgs, arg passed in is: " << std::endl;};
  auto jobWithReturn = [](){ return std::string("returned from jobWithReturn");};
  
  
   /* ================== Creating a Pool ================== */
   
  //the pool will allow you to make any number of threads up to 255
  ThreadPool pool(5); //creates an instance of the threadpool with 5 threads
  //or
  ThreadPool pool2; // Create a default pool with the number of threads on your cpu
  
  /* ================== Adding Jobs to the Queue/Obtaining Return Values ================== */
  
  pool.push(job); //this is how you pass it a job. Easy as that!
  pool.push(jobWithArgs, 1); //this is how you pass it a job with arguments. Easy!
  
  //ThreadPool::push actually returns a future variable for the functions return type that you gave it.
  //To access the returned value just use "auto <varname> = pool.push()" or "future<type> <varname> = pool.push()".
  //Using auto allows you to not need to remember what the return type of the function is.
  auto future = pool.push(jobWithReturn); 
  
  //Now with the future variable from the push function, just use "future.get()" to access the data!
  std::string returnedString = future.get(); //future.get() blocks until the value is ready to be obtained
  
  /* ================== Utility Functions ================== */
  
  //gets the current thread count of the threadpool
  uint8_t numthreads = pool.getThreadCount(); //returns a uint8_t
 
  
  }
```
