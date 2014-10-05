#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <memory>
#include <queue>
#include <vector>
#include <boost/thread.hpp>
#include "priority_queue.h"

namespace my {
typedef std::function<void()> function;

class threadpool;

template<class T>
class Data;
}

class my::threadpool {
private:
    class work_thread; // собственно класс рабочего процесса
    typedef std::shared_ptr<work_thread> th_pointer;

    my::priority_queue<size_t, my::function> fn_container; // контейнер с функциями
    std::vector<th_pointer> th_container; // контейнер с рабочими процессами
    unsigned int const th_count;   // кол-во рабочих процессов
    bool on;

public:
    friend class work_thread;

    class threadpool_exception;

    threadpool();
    threadpool(unsigned int const);
    ~threadpool();
    void stop();

    template<class R, class FN, class... ARGS>
    void add(size_t priority, std::shared_ptr<my::Data<R>> &ReturnData, FN fn, ARGS... args);
    template<class R, class FN, class... ARGS>
    void add(size_t priority, FN fn, ARGS... args);
};

class my::threadpool::work_thread {
private:
    bool on;
    my::threadpool *pool;
    boost::thread thread;

    void exec();

public:
    friend class threadpool;
    work_thread(my::threadpool *_pool);
    ~work_thread();
};

template<class T>
class my::Data
{
private:
    boost::unique_future<T> data;
    bool set;
public:
    friend my::threadpool;
    Data();
    T get();
    boost::unique_future<T> get_future();
};

class my::threadpool::threadpool_exception: public std::exception {
private:
    char const *message;
public:
    threadpool_exception(char const *_message): message(_message){
        ;
    }
    const char* what() const noexcept {
        return message;
    }
};

#include "threadpool.hpp"

#endif // THREADPOOL_H
