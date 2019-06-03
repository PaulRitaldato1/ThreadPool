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
	return 5;
}

std::string dummy1ArgRtn(int i) {
	std::cout << "Arg Passed in: " << i << std::endl;
	return "Hello There";
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

 class task {
 private:
	 std::packaged_task<void()> func;

 public:
	 virtual ~task() {}
	 virtual void execute() = 0;
 };

 template <typename T>
 class AnyTask :  public task {
 private:
	 std::packaged_task<T()> func;
 public:
	 AnyTask(std::packaged_task<T()> func) : func(std::move(func)) {}

	 void execute() {
		 func();
	 }

 };

 class Future {
 public:
	 virtual ~Future() {}
	 std::future<void> future;
 };

 template <typename FutureType>
 class AnyFuture : public Future {
 public:
	 AnyFuture(std::future<FutureType> future) : future(std::move(future)) {}
	 std::future<FutureType> future;
 };

int main(){

	//push_test(dummy1ArgRtn, 1);
	//std::vector<std::shared_ptr<task>> jobs;
	//std::vector<std::shared_ptr<Future>> futures;
	//
	//std::packaged_task<int()> f1([]() {std::cout<<"running this function" << std::endl; return 5; });
	//std::packaged_task<void()> f2([]() {});
	//std::packaged_task<int()> f3(std::move(std::bind(dummy1ArgRtn, 1)));
	//std::future<int> fut1 = f1.get_future();
	//std::future<void> fut2 = f2.get_future();
	//std::future<int> fut3 = f3.get_future();

	//futures.reserve(3);
	//futures.emplace_back(std::make_shared<AnyFuture<int>>(std::move(fut1)));
	//futures.emplace_back(std::make_shared<AnyFuture<void>>(std::move(fut2)));
	//futures.emplace_back(std::make_shared<AnyFuture<int>>(std::move(fut3)));

	////std::function<void()> b1 = std::bind(std::move(f1));

	////std::cout << typeid(fut1).name() << std::endl;
	//jobs.push_back(std::make_shared<AnyTask<int>>(std::move(f1)));
	//jobs.push_back(std::make_shared<AnyTask<void>>(std::move(f2)));
	//jobs.push_back(std::make_shared<AnyTask<int>>(std::move(f3)));	


	////std::packaged_task<int()> j = std::move(std::packaged_task<int()>((*jobs[0]).f.unc));
	//(*jobs[0]).execute();
	////task j = std::move(*jobs[0]);
	////auto j = std::move((*jobs[0]).func);
	////std::cout << typeid(j.func).name() << std::endl;
	////f3();
	//auto& f = std::move(static_cast<AnyFuture<int>&>(*futures[0]));

	//if (f.future.valid())
	//	std::cout << "Getting future value: " << f.future.get() << std::endl;
	//else
	//	std::cout << "not valid" << std::endl;



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

	//test1();

    //test2();

    //test1();
    //must wait. Sometimes the main program will end before the threads finish executing their job. The thrteads then get killed early
    //std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    //test4();
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //test5();
}
