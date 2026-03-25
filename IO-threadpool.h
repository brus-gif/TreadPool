#ifndef  TREADPOOL_H
#define TREADPOOL_H

#include"LockcircleQueue.h"
#include<iostream>
#include<functional>
#include<unordered_map>
#include<future>
#include <type_traits>
#include <memory>
#include <stack>
#include <mutex>
#include <condition_variable>
#include <thread>

class Thread
{
private:
	using threadfun = std::function<void(int)>;
	int id_;
	threadfun fun_;
	static std::atomic_int threadid_;//id计数
	std::thread t;

public:
	Thread(int id, threadfun fun) :fun_(fun), id_(id) {}
	~Thread() { t.join(); }
	void start(int thread_id);//线程开启函数	
	static int Addid();//增加全局id计数
};
class Steward
{
private:
	std::stack<int>st;//id销毁栈
	using Func = std::function<void()>;
	Func func_;
	std::thread t1;

public:
	Steward(Func func);

	Steward() {}
	~Steward() { t1.join(); }
	void start();
	std::stack<int>* getstack();
	void pushid(int id);//往栈中推送id
};

enum class Mode
{
	MODE_FIXED, MODE_COAHCHED
};


class ThreadPool
{
private:
	int wake_;//唤醒标准
	using Task = std::function<void()>;
	std::unique_ptr<Steward> stew;//管家线程对象指针
	CircleQueue<Task>que;
	int task_max_num;//任务队列中任务数量上限
	std::atomic_int curtasknum;//当前队列中的任务数量
	std::atomic_int num;//提交计数

	std::unordered_map<int, std::unique_ptr< Thread>>thread_;//线程列表
	int thread_max_num;//最大线程数量
	std::atomic_int curthreadnum;//当前线程数量
	std::atomic_int curfreethread;//当前空闲线程数量
	int initThreadnum;//初始线程数量

	std::stack<int>id_stack;//id获取栈

	void threadhanderl(int id);//线程函数
	void stewthreadhanderl();//管家线程函数

	Mode mode_;//线程池模式
	std::atomic_bool is_running;//线程池状态

	std::mutex subtex;//任务提交函数锁
	std::mutex tex;//任务执行函数锁
	std::mutex stack_mtx;//栈锁
	std::condition_variable notempty;//队列不空
	std::condition_variable notfull;//队列不满

	std::mutex stx;//管家锁
	std::condition_variable cond;
public:

	ThreadPool(int size);
	~ThreadPool();
	template<typename Fun, typename ...Args>
	auto tasksubmit(Fun&& fun, Args&&...args) -> std::future<std::invoke_result_t<Fun, Args...>>
	{

		using Type = std::invoke_result_t<Fun, Args...>;
		
			auto task = std::make_shared<std::packaged_task<Type()>>(std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...));//task一个在任务执行函数，一个在任务提交函数，
		std::future<Type> result = task->get_future();
		std::unique_lock<std::mutex>lock(subtex);
		if (is_running)
		{
			if (notfull.wait_for(lock, std::chrono::seconds(3), [&]()->bool {return curtasknum < task_max_num; }) == false)
			{
				std::cerr << "the taskqueue is full ,your submition is fall !" << std::endl;
				auto test = std::make_shared<std::packaged_task<Type()>>([]()->Type {return Type(); });
				std::future<Type> fall_result = test->get_future();
				(*test)();
				return fall_result;
			}
			que.enpush([task]()->void {(*task)(); });
			curtasknum++;
			if (num >= wake_ && curfreethread != 0)
			{
				notempty.notify_one();
				num = 0;
			}
			if (1.5 * curthreadnum <= curtasknum)
			{
				notempty.notify_all();
				cond.notify_all();
			}
			return result;
		}
		else
		{
			std::cerr << "the threadpool has closen ,the submition is fall !" << std::endl;
			auto test_t = std::make_shared<std::packaged_task<Type()>>([]()->Type {return Type(); });
			std::future<Type> result_x = test_t->get_future();
			(*test_t)();
			return result_x;
		}

	}
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	void start(int threadnum);//线程池开启函数
	void SetMode(Mode mode);//设置线程池模式
	void setTaskmaxnum(int num);
	void setWake(int wake);
	void setThreadmaxnum(int num);
	void setInitThread(int num);
};

#endif // ! TREADPOOL_H