#ifndef P_QUEUE_H
#define P_QUEUE_H

#include <boost/thread.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include <vector>
#include <memory>
#include <iostream>

namespace my {

template <class KeyT, class ValueT>
class priority_queue;

template <class KeyT, class ValueT>
class node;

template <class T>
void swap(T &a, T &b);
}

template <class KeyT, class ValueT>
class my::priority_queue {
private: public:
    std::vector<my::node<KeyT, ValueT>> heap;
    boost::condition_variable cv;
    boost::shared_mutex sh_mutex;
    boost::shared_mutex heapify_mutex;
    boost::mutex u_mutex;
    bool on;

    size_t parent(size_t i);
    size_t left(size_t i);
    size_t right(size_t i);

    void up_heapify(size_t i); // однопоточный
    void down_heapify(size_t i); // многопоточный
public:
    priority_queue();
    ~priority_queue();

    void put(ValueT const &value, KeyT const &key = 0); // однопоточный
    bool get(ValueT &result); // многопоточный

    void finish();
    bool is_finished();
    bool empty();
};

template <class KeyT, class ValueT>
class my::node {
public:
    node(KeyT k, ValueT v);
    node();
    ~node();

    KeyT key;
    ValueT value;
    boost::detail::spinlock n_mutex;
};


#include "priority_queue.hpp"

#endif //P_QUEUE_H
