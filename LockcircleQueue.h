//#ifndef  CIRCLEQUEUE
//
//#define CIRCLEQUEUE
////#ifndef LOCKQUEUE
//#define LOCKQUEUE
//
//#include <iostream>
//#include <memory>
//#include <mutex>
//#include <atomic>
//
//template<typename T>
//class CircleQueue {
//private:
//    T* arr_;                // 底层数组
//    int capacity_;          // 最大容量
//    std::atomic<int> front_; // 消费者索引
//    std::atomic<int> rear_;  // 生产者索引
//    std::atomic<int> size_;  // 当前任务数
//    std::allocator<T> alloc_;
//
//public:
//    // 构造：一次性分配，不再扩容
//    CircleQueue(int size = 20) 
//        : capacity_(size), front_(0), rear_(0), size_(0) {
//        arr_ = alloc_.allocate(capacity_);
//    }
//
//    // 析构：必须从有效区间销毁对象
//    ~CircleQueue() {
//        int f = front_.load();
//        int s = size_.load();
//        for (int i = 0; i < s; ++i) {
//            std::destroy_at(&arr_[(f + i) % capacity_]);
//        }
//        alloc_.deallocate(arr_, capacity_);
//    }
//
//    // 严禁拷贝（多线程容器不应允许拷贝）
//    CircleQueue(const CircleQueue&) = delete;
//    CircleQueue& operator=(const CircleQueue&) = delete;
//
//    // 入队：由生产者调用（需配合 ThreadPool 的锁）
//    bool enpush(T val) {
//        if (size_.load() >= capacity_) return false;
//
//        int r = rear_.load();
//        // 在原始内存上构造对象
//        std::construct_at(&arr_[r], std::move(val));
//        
//        // 更新索引和大小
//        rear_.store((r + 1) % capacity_);
//        size_++;
//        return true;
//    }
//
//    // 取任务：由消费者调用（需配合 ThreadPool 的锁）
//    T de_front() {
//        int f = front_.load();
//        // 1. 拷贝/移动出对象
//        T val = std::move(arr_[f]);
//        
//        // 2. 物理销毁数组中的旧对象（非常重要！防止内存状态混乱）
//        std::destroy_at(&arr_[f]);
//        
//        // 3. 更新索引
//        front_.store((f + 1) % capacity_);
//        size_--;
//        
//        return val;
//    }
//
//    bool empty() const { return size_.load() == 0; }
//    int size() const { return size_.load(); }
//};
//
//#endif
//
#ifndef LOCKQUEUE
#define LOCKQUEUE

#include <iostream>
#include <memory>
#include <mutex>

template<typename T>
class CircleQueue {
private:
    T* arr_;
    int capacity_;
    int front_;
    int rear_;
    int que_size_;

    std::allocator<T> alloct_;
    std::mutex mtx_; // 内部锁，保护所有队列操作

public:
    explicit CircleQueue(int size = 20):que_size_(0), capacity_(size), front_(0), rear_(0) 
    {
        arr_ = alloct_.allocate(capacity_);
    }

    ~CircleQueue() {
        std::lock_guard<std::mutex> lock(mtx_);
        // 析构时只销毁存在的任务
        for (int i = 0; i < que_size_; ++i) {
            std::destroy_at(&arr_[(front_ + i) % capacity_]);
        }
        alloct_.deallocate(arr_, capacity_);
    }

    // 严禁拷贝
    CircleQueue(const CircleQueue&) = delete;
    CircleQueue& operator=(const CircleQueue&) = delete;

    // 入队：成功返回true，满员返回false
    bool enpush(T val) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (que_size_ >= capacity_) return false;

        // 在原始内存构造新对象
        std::construct_at(&arr_[rear_], std::move(val));
        rear_ = (rear_ + 1) % capacity_;
        que_size_++;
        return true;
    }

    // 出队：成功返回true并通过val传出任务，空队列返回false
    bool depop(T& val) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (que_size_ == 0) return false;

        // 1. 移动出对象
        val = std::move(arr_[front_]);
        // 2. 物理销毁原位置对象，防止内存状态污染
        std::destroy_at(&arr_[front_]);

        front_ = (front_ + 1) % capacity_;
        que_size_--;
        return true;
    }

    int size() {
        std::lock_guard<std::mutex> lock(mtx_);
        return que_size_;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx_);
        return que_size_ == 0;
    }
};

#endif
