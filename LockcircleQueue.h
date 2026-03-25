
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
