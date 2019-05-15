# ThreadPool
I needed an easy to use thread pool for C++ for my game engine project, so here it goes.

## Usage
The ThreadPool class is a Singleton. So here is an example of how to use it.


```
#include "ThreadPool.h"

void job(){
  //do stuff
}

void jobWithArgs(int arg){
  //do stuff
}
int main () {
  
  //the pool will only allow you to make any number of threads as long as its less than your max thread count
  ThreadPool& pool = ThreadPool::getInstance(5); //This creates an instance of the threadpool with 5 thread (if one hasnt been created)
  
  pool.push(job); //this is how you pass it a job. Easy as that!
  
  //Return types are accepted but NOT checked. There is no way to get a return value from these jobs
  //to give a job with arguments (up to 4 arguments are supported) simply call "push" as follows
  pool.push<int>(jobWithArgs, 5); //the "push" function is templated and overloaded so just specify the argument types with push
  
  //general push format
  // pool.push<argtype,argtype,argtype,argtype>(function, arg1, arg2, arg3, arg4);
  }
