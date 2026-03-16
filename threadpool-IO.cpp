#include"threadpool-IO.h"
ThreadPool::ThreadPool(int size=200)
	:stew(new Steward(std::bind(&ThreadPool::stewthreadhanderl,this)))
	, task_max_num(10)
	, curtasknum(0)
	, num(0)
	, thread_max_num(4* std::thread::hardware_concurrency())
	, curthreadnum(0)
	, curfreethread(0)
	,wake_(5)
	, que(size)
	,initThreadnum(4)
	, is_running(false)
	, mode_(Mode::MODE_FIXED)
{}
std::atomic_int Thread::threadid_ = 0;//全局id计数
ThreadPool::~ThreadPool()
{ 
	is_running = false;
	notempty.notify_all();
	cond.notify_all();
	std::unique_lock<std::mutex>lock(tex);
	while (curthreadnum > 0)
	{
		cond.wait_for(lock, std::chrono::seconds(5), [&]()->bool {return curthreadnum == 0; });
	}
}


Steward::Steward(Steward::Func func):func_(func) {}

void ThreadPool::setTaskmaxnum(int num) 
{
	if (is_running)
	{
		std::cerr << "the threadpool is running " << std::endl;
		return; 
	}
	task_max_num = num;
}

void ThreadPool::setWake(int wake) 
{ 
	if (is_running)
	{
		std::cerr << "the threadpool is running " << std::endl;
		return;
	}
	wake_ = wake;
}
void ThreadPool::SetMode(Mode mode) 
{
	if (is_running)
	{
		std::cerr << "the threadpool is running " << std::endl;
		return;
	}
	mode_=mode;
}
void ThreadPool::setThreadmaxnum(int num)
{ 
	if (is_running)
	{
		std::cerr << "the threadpool is running " << std::endl;
		return;
	}
	thread_max_num = num; 
}

void ThreadPool::start(int threadnum)
{
	initThreadnum = threadnum;
	is_running = true;
	for (int i=0;i< threadnum;i++)
	{
		thread_.emplace(i,std::make_unique<Thread>(i,std::bind(&ThreadPool::threadhanderl,this,std::placeholders::_1)));
	}
	stew->start();
	for (int i = 0; i < threadnum; i++)
	{
		thread_[i]->start(i);
		thread_[i]->Addid();
		curthreadnum++;
	}
}
void ThreadPool::threadhanderl(int id)
{
	auto lasttime = std::chrono::high_resolution_clock().now();
	while (is_running || curtasknum > 0)
	{
		Task task;
		{

			std::unique_lock<std::mutex>lock(tex);
			while (curtasknum == 0 && is_running)
			{
				curfreethread++;
				if (notempty.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout && mode_ == Mode::MODE_COAHCHED)
				{
					auto now = std::chrono::high_resolution_clock().now();
					auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lasttime);
					if (dur >= std::chrono::seconds(60) && curthreadnum > initThreadnum)
					{
						std::lock_guard<std::mutex>lock(stack_mtx);
						stew->pushid(id);
						curthreadnum--;
						curfreethread--;
						return;
					}

				}
				curfreethread--;
			}
			if (curtasknum == 0 && !is_running)
			{
				curthreadnum--;
				if (curthreadnum == 0)
				{
					cond.notify_all();//管家与析构函数全部叫醒
				}
				return;

			}
		
		if (que.depop(task)==false)
		{
			curtasknum--;
			continue;
		}
		curtasknum--;
		if (curtasknum == 0)
		{
			notfull.notify_all();
		}
		}
		if (task)
		{
			task();
			lasttime = std::chrono::high_resolution_clock().now();
		}
	}
	curthreadnum--;
}

void ThreadPool::stewthreadhanderl()
{
	while (is_running)
	{
		std::unique_lock<std::mutex>lock(stx);
		if (1.5 * curthreadnum <= curtasknum)
		{
			notempty.notify_all();
			if (mode_ == Mode::MODE_COAHCHED)
			{
				for (int i = 0; i < 0.15 * thread_max_num&& curthreadnum< thread_max_num; i++)
				{
					int cur_id = Thread::Addid();
					thread_.emplace(cur_id, std::make_unique<Thread>(cur_id, std::bind(&ThreadPool::threadhanderl, this, std::placeholders::_1)));
					thread_[cur_id]->start(cur_id);
					curthreadnum++;
					std::cout << "creat new thread!" << std::endl;
				}
			}

		}
		if (curtasknum < curfreethread)
		{
			notfull.notify_one();
		}

		if (mode_ == Mode::MODE_COAHCHED)
		{
			std::unique_lock<std::mutex>lock(stack_mtx);
			std::stack<int>*st=stew->getstack();
			while (!st->empty())
			{
				int get_id = st->top();
				thread_.erase(get_id);
				st->pop();
				std::cout << "destroy the thread!" << std::endl;
			}


		}
		cond.wait_for(lock,std::chrono::seconds(2));
	}
}

void ThreadPool::setInitThread(int num)
{
	if (is_running)
	{
		std::cerr << "the threadpool is running !" << std::endl;
		return;
	}
	initThreadnum = num;
}
void Thread::start(int thread_id)
{
	std::thread t(fun_, thread_id);
	t.detach();
}

void Steward::start()
{
	std::thread t1(func_);
	t1.detach();
}

std::stack<int>* Steward::getstack()
{
	return &st;
}

int Thread::Addid()
{
	return threadid_++;
}

void Steward::pushid(int id)
{
	st.push(id);
}

int sum1(int a, int b)
{
	std::this_thread::sleep_for(std::chrono::seconds(10));
	// 比较耗时
	return a + b;
}
int sum2(int a, int b, int c)
{
	std::this_thread::sleep_for(std::chrono::seconds(10));
	return a + b + c;
}
int main()
{
	ThreadPool pool{};
	pool.SetMode(Mode::MODE_COAHCHED);
	pool.start(1);

	int sum = 0;
	auto start = std::chrono::high_resolution_clock::now();
	std::future<int> r4 = pool.tasksubmit([](int b, int e)->int {
		int sum = 0;
		for (int i = b; i <= e; i++)
			sum += i;
		std::this_thread::sleep_for(std::chrono::seconds(10));
		return sum;
		}, 1, 100);
	std::future<int> r5 = pool.tasksubmit([](int b, int e)->int {
		int sum = 0;
		for (int i = b; i <= e; i++)
			sum += i;
		std::this_thread::sleep_for(std::chrono::seconds(10));

		return sum;
		}, 1, 100);

	std::future<int> r1 = pool.tasksubmit(sum1, 1, 2);
	std::future<int> r2 = pool.tasksubmit(sum2, 1, 2, 3);
	


	std::cout << r1.get() << std::endl;
	std::cout << r2.get() << std::endl;
	std::cout << r4.get() << std::endl;
	std::cout << r5.get() << std::endl;
	auto end = std::chrono::high_resolution_clock::now();
	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
	std::cout << dur << std::endl;
	//std::this_thread::sleep_for(std::chrono::seconds(30));

return 0;
}