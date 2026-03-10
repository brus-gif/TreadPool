#if 0
//#ifndef THREAD_H
//#define THREAD_H
//#include<iostream>
//#include<algorithm>
//#include<queue>
//#include<atomic>
//#include<mutex>
//#include<condition_variable>
//#include<memory>
//#include<functional>
//#include<type_traits>
//#include<thread>
//#include<chrono>
////#include<time.h>
////任务类:
//class Task
//{
//	virtual void run()=0;
//
//};
//
//class baseThread
//{
//public:
//	virtual void start()=0;
//	virtual ~baseThread() = default;
//protected:
//	baseThread() = default;
//};
//
////线程类
//template<typename T>
//class Thread:public baseThread
//{
//private:
//	using threadFunc = std::_Invoke_result_t<T>;
//	std::function<threadFunc()>func;
//public:
//
//	//线程类的构造函数
//	Thread(T fun );
//
//	//线程类构造函数（传入一个绑定器对象）
//	virtual void start();
//};
//
////线程池模式类
//enum class PoolMode
//{
//	MODE_FIXED, MODE_COACHED
//};
//
////线程池类
//class ThreadPool 
//{
//private:
//	std::vector<baseThread*>treads_;//线程池线程列表
//	int initTreadsize_;//初始线程的数量
//
//	std::queue<std::shared_ptr<Task>>que;//任务队列
//	std::atomic_int Tasksize_;//当前队列中任务的数量
//	int Taskquemaxsize_;//任务队列上限的的阈值
//
//	std::mutex taskquetex_;//任务队列安全锁（）
//	std::condition_variable notfull;//队列不满
//	std::condition_variable notempty;//队列不空
//
//	PoolMode Mode_;//线程池模式
//private:
//	void treadhandler(int thread_id);
//public:
//	ThreadPool();
//	~ThreadPool();
//	void start(int initTreadsize);//开启线程池
//	void setPoolMode(PoolMode Mode);//设置线程池模式
//	void submitTask(std::shared_ptr<Task>task);//提交任务(任务一般在堆上建立)
//	void setTaskquemaxsize(int Taskquemaxsize);//设置任务队列的上限阈值
//	void setinitTreadsize(int initTreadsize);//设置初始线程数量
////删除拷贝构造和赋值重载
//	ThreadPool(ThreadPool&) = delete;
//	ThreadPool& operator=(const ThreadPool&) = delete;
//
//};
//
//



#endif

#endif