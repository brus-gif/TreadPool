<<<<<<< HEAD
#define MAX_QUE_SIZE 2
#include "2threadpool.h"


ThreadPool::ThreadPool()
	: initTreadsize_(4),
	curfreethreadnum(0),
	state(true),
	Tasksize_(0),
	Taskquemaxsize_(MAX_QUE_SIZE),
	Mode_(PoolMode::MODE_FIXED),
	curthreadnum(4),
	maxThreadsize_(MAXTHREAD),
	stew(new Steward(std::bind(&ThreadPool::stewardhandler,this)))
{}
ThreadPool::~ThreadPool() 
{
	state = false;
	notempty.notify_all();
	std::unique_lock<std::mutex>lock(taskquetex_);
	while (curthreadnum>0)
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
		treads_.emplace(i,std::make_unique< Thread>(std::bind(&ThreadPool::treadhandler, this,std::placeholders::_1),i));//创建线程放到线程列表

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
//提交任务(任务一般在堆上建立) 
MyResult* ThreadPool::submitTask(Task*task)
{
	MyResult* ptr = task->getresult();
	if(state)
	{
		std::unique_lock<std::mutex>lock(taskquetex_);
		if (notfull.wait_for(lock, std::chrono::seconds(4), [&]()->bool {return Tasksize_ <Taskquemaxsize_; })==false)
		{
			st.notify_one();
			std::cerr << "The submition is false! " << std::endl;
			return ptr;
		}
		que.push(task);
		Tasksize_++;
		std::cout << "success submition!" << std::endl;
			notempty.notify_all();
			if (1.5 * curfreethreadnum < Tasksize_)
			{
				st.notify_one();
			}
	}
	else
	{
		std::cerr << "the threadpool has closen" << std::endl;
	}
	return ptr;
}

//线程函数
void ThreadPool::treadhandler(int thread_id)
{
	auto lasttime = std::chrono::high_resolution_clock().now();

	while (state || Tasksize_ > 0)
	{
		Task* task=nullptr;
		{
			std::unique_lock<std::mutex>lock(taskquetex_);
			while (state&& Tasksize_ ==0)
			{
				curfreethreadnum++;
				if (notempty.wait_for(lock, std::chrono::seconds(1))==std::cv_status::timeout&& Mode_== PoolMode::MODE_COACHED)//待优化点（苏醒后抢锁的时间不包含在时间计数中）
				{
					auto now= std::chrono::high_resolution_clock().now();
					auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lasttime);
					if (dur >= std::chrono::seconds(60)&& curthreadnum > initTreadsize_)//管家线程处理点
					{
						stew->set_destroy_threadid(thread_id);
						curfreethreadnum--;
						curthreadnum--;
						return;
					}
				}
				curfreethreadnum--;
			}
			if (!state && Tasksize_ ==0)
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
		task->exec();
		delete task;
		lasttime = std::chrono::high_resolution_clock().now();
	}
	curthreadnum--;
	curfreethreadnum--;
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
	curfreethreadnum = initTreadsize;
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
std::atomic_int Thread:: calculate_id = 0;
//线程函数的开启
void Thread::start(int id)
{
	std::thread t(func,id);
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

		if (curfreethreadnum <Tasksize_ &&1.5* curfreethreadnum >=Tasksize_)
		{
			notempty.notify_all();
		}
		else if(1.5 * curfreethreadnum <Tasksize_)
		{
			notempty.notify_all();
			for (int i = 0; i < 4; i++)
			{
				int id = Thread::AddId();
				treads_.emplace(id, std::make_unique< Thread>(std::bind(&ThreadPool::treadhandler, this, std::placeholders::_1),id));
				treads_[id]->start(id);
				curthreadnum++;
				std::cout << "add thread" << std::endl;
			}

		}


		std::stack<int>* sck= stew->get_stack();
		while(!sck->empty())
		{
			treads_.erase(sck->top());
			sck->pop();
			std::cout << "steward succeed destroy the thread!" << std::endl;
		}
		st.wait_for(lock,std::chrono::seconds(2));
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


void MyResult::setAny(MyAny&& any)
{	
	final1 =std::move( any);
	sem.post();
}

MyAny MyResult::getAny()
{
	sem.wait_();
	return std::move(final1);
}
void Task::exec()
{
	result->setAny(std::move(run()));
	std::cout << "success excute!" << std::endl;

}
MyResult* Task::getresult()
{
	return result;
}
void ThreadPool::setStae(bool state_)
{
	state = state_;
}

