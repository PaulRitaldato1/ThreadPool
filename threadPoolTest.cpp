#include "ThreadPool.h"
#include <chrono>
#include <string>
#include <iostream>

ThreadPool pool(5);
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
	std::string s = "Hello There";
	return s;
}
void blah(int& i) {

	i += 10;
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

 int busy() {
	
	 uint64_t ret = 0;;
	 for (uint32_t i = 0; i != 100000000; ++i) {
		 ret *= i;
	 }
	 return ret;
 }

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
 /*busy();
busy();
busy();
busy();
 busy();*/

	auto start = std::chrono::high_resolution_clock::now();
	uint64_t r1 = busy();
	uint64_t r2 = busy();
	uint64_t r3 = busy();
	uint64_t r4 = busy();
	uint64_t r5 = busy();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << duration.count() << std::endl;

	 start = std::chrono::high_resolution_clock::now();
	auto ret1 = pool.push(busy);
	auto ret2 = pool.push(busy);
	auto ret3 = pool.push(busy);
	auto ret4 = pool.push(busy);
	auto ret5 = pool.push(busy);

	ret1.get();
	ret2.get();
	ret3.get();
	ret4.get();
	ret5.get();
	 stop = std::chrono::high_resolution_clock::now();
	 duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << duration.count() << std::endl;

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
	return 0;
}
