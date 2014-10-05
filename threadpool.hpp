#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "threadpool.h"

template<class T>
T my::Data<T>::get() {
    while(!set);
    data.wait();
    return data.get();
}

template<class T>
boost::unique_future<T> my::Data<T>::get_future() {
    while(!set);
    return data;
}

template<class T>
my::Data<T>::Data(): set(false) {
    ;
}

////////////////////threadpool/////////////////////////

my::threadpool::threadpool(): fn_container(), th_container(), th_count(boost::thread::hardware_concurrency())  //как определить оптимальное число?
{
    for (unsigned int i = 0; i < th_count; i++) {
        th_pointer worker(new work_thread(this));
        th_container.push_back(worker);
    }
}

my::threadpool::threadpool(const unsigned int th_num): fn_container(), th_container(), th_count(th_num)
{
    if (th_count == 0) {
        throw my::threadpool::threadpool_exception("too small number of threads");
    }
    for (unsigned int i = 0; i < th_count; i++) {
        th_pointer worker(new work_thread(this));
        th_container.push_back(worker);
    }
}

my::threadpool::~threadpool(){
    stop();
}

void my::threadpool::stop() {
    fn_container.finish();
    while (!fn_container.is_finished()) {
        ;
    }
    for (unsigned int i = 0; i < th_count; i++) {
        th_container[i]->on = false;
    }
}

template<class R, class FN, class... ARGS>
void my::threadpool::add(size_t priority, std::shared_ptr<my::Data<R>> &ReturnData, FN fn, ARGS... args) {
    std::function<R()> rfn = std::bind(fn, args...);
    function pool_fn = [=]() {
        boost::packaged_task<R> pt(rfn);
        ReturnData->data = pt.get_future();
        ReturnData->set = true;
        pt();
    };
    fn_container.put(pool_fn, priority);
}

template<class R, class FN, class... ARGS>
void my::threadpool::add(size_t priority, FN fn, ARGS... args) {
    function rfn = std::bind(fn, args...);
    fn_container.put(rfn, priority);
}

////////////////////work_thread/////////////////////////

my::threadpool::work_thread::work_thread(my::threadpool *_pool):
    on(true),
    pool(_pool),
    thread(&my::threadpool::work_thread::exec, this) {
    ;
}

my::threadpool::work_thread::~work_thread() {
    thread.join();
}

void my::threadpool::work_thread::exec() {
    function current;
    while (on) {
        if (pool->fn_container.get(current))
            current();
        else break;
    }
}


#endif //THREADPOOL_HPP
