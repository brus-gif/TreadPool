#define MAX_QUE_SIZE 2
#include "threadpool.h"


ThreadPool::ThreadPool()
	: initTreadsize_(4),
	curfreethreadnum(0),
	state(true),
	Tasksize_(0),
	Taskquemaxsize_(MAX_QUE_SIZE),
	Mode_(PoolMode::MODE_FIXED),
	curthreadnum(4),
	maxThreadsize_(MAXTHREAD),
	stew(new Steward(std::bind(&ThreadPool::stewardhandler, this)))
{}
ThreadPool::~ThreadPool()
{
	state = false;
	notempty.notify_all();
	std::unique_lock<std::mutex>lock(taskquetex_);
	delete stew;
	while (curthreadnum > 0)
	{
		cond.wait_for(lock, std::chrono::seconds(5), [&]()->bool {return curthreadnum == 0; });
	}
}
//开启线程池
void ThreadPool::start(int initTreadsize)
{
	initTreadsize_ = initTreadsize;//传入初始线程数量
	curthreadnum = initTreadsize;//传入当前线程数量
	for (int i = 0; i < initTreadsize; i++)
	{
		treads_.emplace(i, std::make_unique< Thread>(std::bind(&ThreadPool::treadhandler, this, std::placeholders::_1), i));//创建线程放到线程列表

	}
	stew->start();//管家线程启动

	for (int i = 0; i < initTreadsize; i++)
	{
		treads_[i]->start(i);//开启所有线程
		treads_[i]->AddId();

	}

}
//设置线程池模式
void ThreadPool::setPoolMode(PoolMode Mode)
{
	Mode_ = Mode;
}

//线程函数
void ThreadPool::treadhandler(int thread_id)
{
	auto lasttime = std::chrono::high_resolution_clock().now();

	while (state || Tasksize_ > 0)
	{
		Task task;
		{
			std::unique_lock<std::mutex>lock(taskquetex_);
			while (state && Tasksize_ == 0)
			{
				curfreethreadnum++;
				if (notempty.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout && Mode_ == PoolMode::MODE_COACHED)//待优化点（苏醒后抢锁的时间不包含在时间计数中）
				{
					auto now = std::chrono::high_resolution_clock().now();
					auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lasttime);
					if (dur >= std::chrono::seconds(60) && curthreadnum > initTreadsize_)//管家线程处理点
					{
						stew->set_destroy_threadid(thread_id);
						curfreethreadnum--;
						curthreadnum--;
						return;
					}
				}
				curfreethreadnum--;
			}
			if (!state && Tasksize_ == 0)
			{
				curthreadnum--;
				if (curthreadnum == 0)
				{
					cond.notify_all();
				}
				return;
			}
			 task = que.front();
			que.pop();
			Tasksize_--;
			notfull.notify_all();
		}
		//task->exec();
		//delete task;
		(task)();
		lasttime = std::chrono::high_resolution_clock().now();
	}
	curthreadnum--;
}




//设置任务队列的上限阈值
void ThreadPool::setTaskquemaxsize(int Taskquemaxsize)
{
	if (state)
	{
		std::cerr << " the threadpool is working ,can't change setTaskquemaxsize" << std::endl;
		return;
	}
	Taskquemaxsize_ = Taskquemaxsize;
}
void ThreadPool::setinitTreadsize(int initTreadsize)
{
	if (state)
	{
		std::cerr << " the threadpool is working ,can't change setinitTreadsize" << std::endl;
		return;
	}
	initTreadsize_ = initTreadsize;
	curthreadnum = initTreadsize;
}
Thread::Thread(threadFunc fun, int id)
{
	func = fun;
	threadid = id;
}
int Thread::AddId()
{
	return calculate_id++;
}
std::atomic_int Thread::calculate_id = 0;
//线程函数的开启
void Thread::start(int id)
{
	std::thread t(func, id);
	t.detach();
}
int Thread::getid()
{
	return calculate_id;
}

//MyAny::MyAny(MyAny&&val) { base = std::move(val.base); }
//MyAny& MyAny::operator=(MyAny&& val) { base =std::move( val.base); return *this; }

//管家线程函数
void ThreadPool::stewardhandler()
{
	while (state)
	{
		std::unique_lock<std::mutex>lock(steconitor);
		
		 if (1.5 * curfreethreadnum < Tasksize_)
		{
			notempty.notify_all();
			if(Mode_== PoolMode::MODE_COACHED)
			{
				for (int i = 0; i < 4; i++)
				{
				int id = Thread::AddId();
				treads_.emplace(id, std::make_unique< Thread>(std::bind(&ThreadPool::treadhandler, this, std::placeholders::_1), id));
				treads_[id]->start(id);
				curthreadnum++;
				std::cout << "add thread" << std::endl;
				}
			
			}
			

		}

		 if (Mode_ == PoolMode::MODE_COACHED)
		 {
			 std::stack<int>* sck = stew->get_stack();
			 while (!sck->empty())
			 {
				 treads_.erase(sck->top());
				 sck->pop();
				 std::cout << "steward succeed destroy the thread!" << std::endl;
			 }
		 }
		st.wait_for(lock, std::chrono::seconds(2));
		
		
	}


}
std::stack<int>* Steward::get_stack()
{
	return &sk;

}

//管家线程的开启
void Steward::start()
{
	std::thread t(func);
	t.detach();
}

void Steward::set_destroy_threadid(int thread_id)
{
	std::unique_lock<std::mutex>lock(stex);
	sk.push(thread_id);

}
void ThreadPool::setStae(bool state_)
{
	state = state_;
}

//#if 0
//// 线程池项目-最终版.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
////
//
//#include <iostream>
//#include <functional>
//#include <thread>
//#include <future>
//#include <chrono>
//using namespace std;
//
//#include "threadpool.h"
//
//
///*
//如何能让线程池提交任务更加方便
//1. pool.submitTask(sum1, 10, 20);
//   pool.submitTask(sum2, 1 ,2, 3);
//   submitTask:可变参模板编程
//
//2. 我们自己造了一个Result以及相关的类型，代码挺多
//	C++11 线程库   thread   packaged_task(function函数对象)  async
//   使用future来代替Result节省线程池代码
//*/
//
//int sum1(int a, int b)
//{
//	this_thread::sleep_for(chrono::seconds(2));
//	// 比较耗时
//	return a + b;
//}
//int sum2(int a, int b, int c)
//{
//	this_thread::sleep_for(chrono::seconds(2));
//	return a + b + c;
//}
//// io线程 
//void io_thread(int listenfd)
//{
//
//}
//// worker线程
//void worker_thread(int clientfd)
//{
//
//}
int main()
{
	ThreadPool pool{};
	// pool.setMode(PoolMode::MODE_CACHED);
	pool.start(2);

	int sum = 0;

std::future<int> r4 = pool.submitTask([](int b, int e)->int {
	int sum = 0;
	for (int i = b; i <= e; i++)
		sum += i;
	return sum;
	}, 1, 100);
std::future<int> r5 = pool.submitTask([](int b, int e)->int {
	int sum = 0;
	for (int i = b; i <= e; i++)
		sum += i;
	return sum;
	}, 1, 100);

	/*std::future<int> r1 = pool.submitTask(sum1, 1, 2);
	std::future<int> r2 = pool.submitTask(sum2, 1, 2, 3);
	std::future<int> r3 = pool.submitTask([](int b, int e)->int {
		future<int> r4 = pool.submitTask(sum1, 1, 2);*/


	//std::cout << r1.get() << std::endl;
	//std::cout << r2.get() << std::endl;
	//std::cout << r3.get() << std::endl;
	std::cout << r4.get() << std::endl;
	std::cout << r5.get() << std::endl;

	//packaged_task<int(int, int)> task(sum1);
	//// future <=> Result
	//future<int> res = task.get_future();
	//// task(10, 20);
	//thread t(std::move(task), 10, 20);
	//t.detach();

	//cout << res.get() << endl;

	/*thread t1(sum1, 10, 20);
	thread t2(sum2, 1, 2, 3);

	t1.join();
	t2.join();*/
}
//#endif
