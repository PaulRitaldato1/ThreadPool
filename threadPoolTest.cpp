#include "ThreadPool.h"
#include <chrono>
#include <string>
ThreadPool& pool = ThreadPool::getInstance(15);

static int counts[5] = {0};
/* dummy jobs to pass to the pool*/
void dummyNoArg(){
    std::cout << "Dummy no arg printing" << std::endl;
    ++counts[0];

}

void dummy1Arg(int i){
    std::string print = "Dummy " + std::to_string(i) + " arg printing\n";
    std::cout << print;
    ++counts[1];

}

void dummy2Arg(int i, int j){
    std::string print = "Dummy " + std::to_string(i) + " arg printing\n";
    std::cout << print;
    ++counts[2];

}

void dummy3Arg(int i, int j, int k){
    std::string print = "Dummy " + std::to_string(i) + " arg printing\n";
    std::cout << print;
    ++counts[3];

}

void dummy4Arg(int i, int j, int k, int l){
    std::string print = "Dummy " + std::to_string(i) + " arg printing\n";
    std::cout << print;

    ++counts[4];

}


/* tests*/
 void test1(){
     pool.push(dummyNoArg);
     pool.push(dummy1Arg, 1);
     pool.push(dummy2Arg, 2, 2);
     pool.push(dummy3Arg, 3, 3, 3);
     pool.push(dummy4Arg, 4, 4, 4, 4);

 }

 void test2(){
     pool.resize(1);
     if(pool.getThreadCount() == 1){
         std::cout << "Resize test passed" <<std::endl;
         return;
     }
     std::cout << "Resize failed" <<std::endl;
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
