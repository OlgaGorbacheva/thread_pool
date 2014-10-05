#ifndef P_QUEUE_CPP
#define P_QUEUE_CPP

#include "priority_queue.h"

template <class T>
void my::swap(T &a, T &b) {
    T c;
    c = a;
    a = b;
    b = c;
    c.n_mutex.unlock();
}

//////////////////////////////////node/////////////////////////////////////

template <class KeyT, class ValueT>
my::node<KeyT, ValueT>::node() {
    n_mutex.unlock();
}

template <class KeyT, class ValueT>
my::node<KeyT, ValueT>::node(KeyT k, ValueT v) {
    key = k;
    value = v;
    n_mutex.unlock();
}

template <class KeyT, class ValueT>
my::node<KeyT, ValueT>::~node() {
    boost::lock_guard<boost::detail::spinlock> lock(n_mutex);
}

//////////////////////////////////queue//////////////////////////////////

template <class KeyT, class ValueT>
my::priority_queue<KeyT, ValueT>::priority_queue(): on(true) {
    ;
}

template <class KeyT, class ValueT>
my::priority_queue<KeyT, ValueT>::~priority_queue(){
    ;
}

template <class KeyT, class ValueT>
size_t my::priority_queue<KeyT, ValueT>::parent(size_t i) {
    if (i > 0)
        return (i - 1) / 2;
    return 0;
}

template <class KeyT, class ValueT>
size_t my::priority_queue<KeyT, ValueT>::left(size_t i) {
    if ((i + 1) * 2 - 1 < heap.size())
        return (i + 1) * 2 - 1;
    return i;
}

template <class KeyT, class ValueT>
size_t my::priority_queue<KeyT, ValueT>::right(size_t i) {
    if ((i + 1) * 2 < heap.size())
        return (i + 1) * 2;
    return i;
}

template <class KeyT, class ValueT>
void my::priority_queue<KeyT, ValueT>::down_heapify(size_t i) {
    boost::unique_lock<boost::detail::spinlock> lock(heap[i].n_mutex);
    if (i != left(i)){
        bool good_choise = false;
        boost::unique_lock<boost::detail::spinlock> left_lock(heap[left(i)].n_mutex);
        if (i != right(i)) {
            boost::unique_lock<boost::detail::spinlock> right_lock(heap[right(i)].n_mutex);
            if (heap[left(i)].key > heap[right(i)].key)
                good_choise = true;
        } else good_choise = true;
        if (heap[i].key < heap[left(i)].key && good_choise) {
            my::swap(heap[i], heap[left(i)]);
            lock.unlock();
            left_lock.unlock();
            down_heapify(left(i));
            return;
        }
        left_lock.unlock();
    }
    if (i != right(i)) {
        boost::unique_lock<boost::detail::spinlock> right_lock(heap[right(i)].n_mutex);
        if (heap[i].key < heap[right(i)].key) {
            my::swap(heap[i], heap[right(i)]);
            lock.unlock();
            right_lock.unlock();
            down_heapify(right(i));
            return;
        }
        right_lock.unlock();
    }
    lock.unlock();
}

//template <class KeyT, class ValueT>
//void my::priority_queue<KeyT, ValueT>::up_heapify(size_t i) {

//}

template <class KeyT, class ValueT>
void my::priority_queue<KeyT, ValueT>::put(const ValueT &value, const KeyT &key) {
    boost::unique_lock<boost::shared_mutex> lock(sh_mutex);
    node<KeyT, ValueT> new_n(key, value);
    heap.push_back(new_n);
    int i = heap.size() - 1;
    while (true) {
        if (i != parent(i) && heap[i].key > heap[parent(i)].key){
            my::swap(heap[i], heap[parent(i)]);
            i = parent(i);
        } else {
            break;
        }
    }
    cv.notify_one();
}

template <class KeyT, class ValueT>
bool my::priority_queue<KeyT, ValueT>::get(ValueT& result) {
    boost::unique_lock<boost::mutex> u_lock(u_mutex);
    cv.wait(u_lock, [this]()->bool {
        return !heap.empty() || !on;
    });
    boost::shared_lock<boost::shared_mutex> sh_lock(sh_mutex);
    boost::unique_lock<boost::shared_mutex> h_lock(heapify_mutex);
    if (!heap.empty()) {
        if ((heap.size() > 1))
            my::swap(heap[0], heap[heap.size() - 1]);
        result = heap[heap.size() - 1].value;
        heap.pop_back();
        int i = 0;
        boost::unique_lock<boost::detail::spinlock> lock(heap[i].n_mutex);
        if (i != left(i)){
            bool good_choise = false;
            boost::unique_lock<boost::detail::spinlock> left_lock(heap[left(i)].n_mutex);
            if (i != right(i)) {
                boost::unique_lock<boost::detail::spinlock> right_lock(heap[right(i)].n_mutex);
                if (heap[left(i)].key > heap[right(i)].key)
                    good_choise = true;
            } else good_choise = true;
            if (heap[i].key < heap[left(i)].key && good_choise) {
                my::swap(heap[i], heap[left(i)]);
                lock.unlock();
                left_lock.unlock();
                u_lock.unlock();
                h_lock.unlock();
                boost::shared_lock<boost::shared_mutex> sh_h_lock(heapify_mutex);
                down_heapify(left(i));
                return true;
            }
        }
        if (i != right(i)) {
            boost::unique_lock<boost::detail::spinlock> right_lock(heap[right(i)].n_mutex);
            if (heap[i].key < heap[right(i)].key) {
                my::swap(heap[i], heap[right(i)]);
                lock.unlock();
                right_lock.unlock();
                u_lock.unlock();
                h_lock.unlock();
                boost::shared_lock<boost::shared_mutex> sh_h_lock(heapify_mutex);
                down_heapify(right(i));
                return true;
            }
        }
        return true;
    }
    return false;
}

template <class KeyT, class ValueT>
void my::priority_queue<KeyT, ValueT>::finish() {
    boost::unique_lock<boost::mutex> lock(u_mutex);
    on = false;
    cv.notify_all();
}

template <class KeyT, class ValueT>
bool my::priority_queue<KeyT, ValueT>::is_finished() {
    boost::unique_lock<boost::mutex> lock(u_mutex);
    return !on && heap.empty();
}

template <class KeyT, class ValueT>
bool my::priority_queue<KeyT, ValueT>::empty() {
    return heap.empty();
};

#endif //P_QUEUE_CPP
