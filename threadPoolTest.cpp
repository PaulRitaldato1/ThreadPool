#include "ThreadPool.h"
#include<chrono>

ThreadPool& pool = ThreadPool::getInstance(15);

static int counts[5] = {0};
/* dummy jobs to pass to the pool*/
void dummyNoArg(){
    std::cout << "Dummy no arg printing" << std::endl;
    ++counts[0];

}

void dummy1Arg(int i){
    std::cout << "Dummy " << i << " arg printing" << std::endl;
    ++counts[1];

}

void dummy2Arg(int i, int j){
    std::cout << "Dummy 2 arg printing" << std::endl;

    ++counts[2];

}

void dummy3Arg(int i, int j, int k){
    std::cout << "Dummy 3 arg printing" << std::endl;
    ++counts[3];

}

void dummy4Arg(int i, int j, int k, int l){
    std::cout << "Dummy 4 arg printing" << std::endl;

    ++counts[4];

}


/* tests*/
 void test1(){
     pool.push(dummyNoArg);
     pool.push<int>(dummy1Arg, 1);
     pool.push<int, int>(dummy2Arg, 2, 2);
     pool.push<int,int,int>(dummy3Arg, 3, 3, 3);
     pool.push<int,int,int,int>(dummy4Arg, 4, 4, 4, 4);

 }

 void test2(){
     pool.resize(1);
 }

 void test4(){
    for(int count : counts){
        if(count != 2){
            std::cout << "Failed test 4" << std::endl;
            return;
        }
    }
    std::cout << "Test 4 Pass" << std::endl;
 }
int main(){

    test1();

    test2();

    test1();
    
    //must wait. Sometimes the main program will end before the threads finish executing their job. The thrteads then get killed early
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    test4();

}
