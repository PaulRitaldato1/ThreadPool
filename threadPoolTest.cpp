#include "ThreadPool.h"
ThreadPool& pool = ThreadPool::getInstance(3);
/* dummy jobs to pass to */
void dummyNoArg(){
    std::cout << "Dummy no arg printing" << std::endl;
}

void dummy1Arg(int i){
    std::cout << "Dummy " << i << " arg printing" << std::endl;
}

void dummy2Arg(int i, int j){
    std::cout << "Dummy 2 arg printing" << std::endl;
}

void dummy3Arg(int i, int j, int k){
    std::cout << "Dummy 3 arg printing" << std::endl;
}

void dummy4Arg(int i, int j, int k, int l){
    std::cout << "Dummy 4 arg printing" << std::endl;
}


/* tests*/
void test1(){
    ThreadPool& pool2 = ThreadPool::getInstance(1);
    if (&pool == &pool2){
        std::cout << "Test 1 passed" << std::endl;
        return;

    }
    std::cout << "Test 1 failed" << std::endl;
}

void test2(){
    ThreadPool& pool2 = ThreadPool::getInstance();
    if (&pool == &pool2){
        std::cout << "Test 2 passed" << std::endl;
        return;
    }
    std::cout << "Test 2 failed" << std::endl;
}

 void test3(){
     pool.push_back(dummyNoArg);
     pool.push_back<int>(dummy1Arg, 1);
     pool.push_back<int, int>(dummy2Arg, 2, 2);
     pool.push_back<int,int,int>(dummy3Arg, 3, 3, 3);
     pool.push_back<int,int,int,int>(dummy4Arg, 4, 4, 4, 4);
 }
int main(){

    //pool.push_back(dummyNoArg);
    //test1();
    //test2();
    test3();
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

}