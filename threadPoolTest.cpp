#include "ThreadPool.h"
#include <chrono>
#include <string>
ThreadPool& pool = ThreadPool::getInstance(7);
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

int dummyRtn() {
	std::cout << "in rtn" << std::endl;
	return 1;
}
void blah(int& i) {

	i += 10;
}
/* tests*/
 void test1(){
     pool.push(dummyNoArg);
     pool.push(dummy1Arg, 1);
     pool.push(dummy2Arg, 2, 2);
     pool.push(dummy3Arg, 3, 3, 3);
     pool.push(dummy4Arg, 4, 4, 4, 4);

	 pool.push(dummyRtn);

	 int i = 5;
	 auto f = [](int &i) {i += 10; };
	 pool.push(blah, i);
	 std::this_thread::sleep_for(std::chrono::milliseconds(500));

	 std::cout << i << std::endl;
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

 void test5(){

     try{
        pool.resize(4);
     }
     catch(bad_thread_alloc& e){
         std::cout << e.what() << std::endl;
     }

 }

 void foo() {

	 auto test = std::bind(dummyRtn);
	 auto lol = test();
	 std::cout << lol << std::endl;

 }



int main(){

	std::vector<std::shared_ptr<task>> jobs;
	
	
	std::packaged_task<int()> f1([]() {std::cout<<"running this function" << std::endl; return 5; });
	std::packaged_task<void()> f2([]() {});
	jobs.push_back(std::make_shared<AnyTask<std::packaged_task<int()>>>(std::move(f1)));
	jobs.push_back(std::make_shared<AnyTask<std::packaged_task<void()>>>(std::move(f2)));

	//std::packaged_task<int()> j = std::move(std::packaged_task<int()>((*jobs[0]).f.unc));
	auto& j = std::move(dynamic_cast<AnyTask<std::packaged_task<int()>>&>(*jobs[0]));
	//auto j = std::move((*jobs[0]).func);
	//std::cout << typeid(j.func).name() << std::endl;
	std::cout << j.func();


	//std::shared_ptr<std::packaged_task<void()>> j = (std::shared_ptr<std::packaged_task<void()>>)jobs[1];
	
	//std::vector<std::future<int()>> futures;
	//std::queue<std::packaged_task<int()>> q;
	//std::packaged_task<int()> f(foo);
	//auto fut = f.get_future();
	//q.push(std::move(f));

	//std::thread th(std::move(q.front()));
	//int res = fut.get();
	//std::packaged_task<void()> j = std::move(q.front());
	//auto test = j();

	//foo();

   // test1();

    //test2();

    //test1();
    //must wait. Sometimes the main program will end before the threads finish executing their job. The thrteads then get killed early
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //test4();
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //test5();
}
