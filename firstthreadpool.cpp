#if 0
//#define MAX_QUE_SIZE 1024
//#include "fristthreadpool.h"
//
//
//ThreadPool::ThreadPool()
//	: initTreadsize_(4),
//Tasksize_(0),
//Taskquemaxsize_(MAX_QUE_SIZE),
//Mode_(PoolMode::MODE_FIXED)
//{}
//ThreadPool::~ThreadPool() {}
////开启线程池
//void ThreadPool:: start(int initTreadsize)
//{
//	initTreadsize_ = initTreadsize;//传入初始线程数量
//	for (int i = 0; i < initTreadsize; i++)
//	{
//		treads_.emplace_back(new Thread<decltype(std::bind(&ThreadPool::treadhandler, this,i))>
//			(std::bind(&ThreadPool::treadhandler,this,i)));//创建线程放到线程列表
//
//	}
//
//	for (int i = 0; i < initTreadsize; i++)
//	{
//		treads_[i]->start();//开启所有线程
//	}
//
//}
////设置线程池模式
//void ThreadPool::setPoolMode(PoolMode Mode)
//{
//	Mode_ = Mode;
//}
////提交任务(任务一般在堆上建立) 
//void ThreadPool::submitTask(std::shared_ptr<Task>task)
//{
//
//}
////设置任务队列的上限阈值
//void ThreadPool::setTaskquemaxsize(int Taskquemaxsize) 
//{
//	Taskquemaxsize_ = Taskquemaxsize;
//}
//void ThreadPool::setinitTreadsize(int initTreadsize)
//{
//	initTreadsize_ = initTreadsize;
//}	
//void ThreadPool::treadhandler(int thread_id) 
//{
//	std::cout << "begin threadfunc" << std::endl;
//}
//template<typename T>
//Thread<T>::Thread(T fun)
//{
//	func = fun;
//}
//template<typename T>
//void Thread<T>::start()
//{
//	std::thread t(func);
//	t.detach();
//}
//int main()
//{
//	ThreadPool pool{};
//	pool.start(4);
//
//	std::this_thread::sleep_for(std::chrono::seconds(5));
//}
#endif
