<<<<<<< HEAD
#ifndef THREAD_H
#define THREAD_H

#include<iostream>
#include<algorithm>
#include<queue>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<memory>
#include<functional>
#include<type_traits>
#include<thread>
#include<chrono>
#include<unordered_map>
#include<stack>
//#include<time.h>

#define MAXTHREAD (28)//最大线程数量
class MyAny;
class Mysemaphore;
//创建管家类：
class Steward
{
private:
	std::stack<int>sk;//存储待销毁的线程id
	using stewardFunc = std::function<void()>;
	stewardFunc func;
	std::mutex stex;
public:
	Steward() {}
	Steward(stewardFunc func_):func(func_) {}
	~Steward(){}
	void start();
	void set_destroy_threadid(int thread_id);//提交待销毁线程id
	std::stack<int>* get_stack();//销毁对应id的线程并清理vec
};

//创建Result类：

//创建Any类
class MyAny 
{
public:
	MyAny() {}
	MyAny(MyAny&)=delete;//
	MyAny(MyAny&&)=default;//
	MyAny& operator=( const MyAny&) = delete;
	MyAny& operator=( MyAny&&)=default;
template<typename T>
MyAny(T data1) :base(new Basechild<T>(data1)){}
~MyAny() {}
template<typename T>
	T cast_()
	{
		Basechild<T>* child = dynamic_cast<Basechild<T>*>(base.get());
		return child->data;
	}
private:
class Base 
	{
	public:
		Base() {}
		virtual ~Base() {}
	};
	template<typename T>
	class Basechild:public Base
	{
	public:
		Basechild() {}
		Basechild(T data_):data(data_){}
		Basechild& operator=(Basechild&&) = default;
		~Basechild() {}
		T data;
	};
	
	std::unique_ptr<Base> base;
};

//semaphore信号量类的实现
class Mysemaphore
{
public:

	Mysemaphore(int num=0)
		:cnt(num)
	{}
	void post() 
	{
		std::unique_lock<std::mutex>lock(setx);
		cnt++;
		se.notify_one();
	}
	void wait_() 
	{
		std::unique_lock<std::mutex>lock(setx);
		se.wait(lock, [&]()->bool {return cnt > 0; });
		cnt--;
	}
private:
	std::condition_variable se;
	std::mutex setx;
	std::atomic<int>cnt;
};
class MyResult
{
public:
	MyResult() {}
	~MyResult() = default;
	MyResult(MyResult&final_)=delete;
	MyResult(MyResult&&final_)=default;
	MyResult& operator=(const MyResult&final_)=delete;
	MyResult& operator = (MyResult&&)=default;
	void setAny(MyAny&& any);
	MyAny getAny();
private:
	MyAny final1;
	Mysemaphore sem;

};

//任务类:
class Task
{
public:
	Task() {}
	Task(MyResult*result_):result(result_){}
	virtual ~Task() { }
	virtual MyAny run() = 0;
	void exec();
	MyResult* getresult();
private:
	MyResult* result;
};

//线程类
class Thread
{
private:
	using threadFunc = std::function<void(const int)>;
threadFunc func;
int threadid;
static std::atomic_int calculate_id;
public:
	

	//线程类的构造函数
	Thread(threadFunc fun,int id);

	//线程类构造函数（传入一个绑定器对象）
	 void start(int id);
	static int AddId();
	 int getid();
};

//线程池模式类
enum class PoolMode
{
	MODE_FIXED, MODE_COACHED
};

//线程池类
class ThreadPool
{
private:
	Steward *stew;//管家类对象指针
	std::atomic_int cnt;//记录提交次数

	std::unordered_map<int,std::unique_ptr<Thread>>treads_;//线程池线程列表
	int initTreadsize_;//初始线程的数量
	std::atomic_int curthreadnum;//当前线程数量
	std::atomic_int curfreethreadnum;//当前空闲线程数量
	int maxThreadsize_;//线程数量的上限阈值

	std::queue<Task*>que;//任务队列
	std::atomic_int Tasksize_;//当前队列中任务的数量
	int Taskquemaxsize_;//任务队列上限的的阈值

	std::mutex taskquetex_;//任务队列安全锁（）
	std::condition_variable notfull;//队列不满
	std::condition_variable notempty;//队列不空

	PoolMode Mode_;//线程池模式
	std::atomic_bool state;//记录线程池状态（开启，关闭）
	std::condition_variable cond;//线程池析构条件变量
	
	std::condition_variable st;//管家线程监视
	std::mutex steconitor;//管家监视锁
private:
	void treadhandler(int thread_id);//线程函数
	void stewardhandler();//管家线程函数
public:
	ThreadPool();
	~ThreadPool();
	void start(int initTreadsize);//开启线程池
	void setPoolMode(PoolMode Mode);//设置线程池模式
	MyResult* submitTask(Task*task);//提交任务(任务一般在堆上建立)
	void setTaskquemaxsize(int Taskquemaxsize);//设置任务队列的上限阈值
	void setinitTreadsize(int initTreadsize);//设置初始线程数量
	//删除拷贝构造和赋值重载
	ThreadPool(ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	void setStae(bool state_);
};
#endif
