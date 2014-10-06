#ifndef QUEUE_H
#define QUEUE_H

#include <queue>
#include <boost/thread.hpp>

namespace my {
template <class T>
class queue;
}

template <class T>
class my::queue {
private: public:
    std::queue<T> q;
    boost::condition_variable cv;
    boost::mutex mutex;
    bool on;
public:

    queue(): on(true) {
        ;
    }

    void put(T const &value) {
        boost::unique_lock<boost::mutex> lock(mutex);
        q.push(value);
        cv.notify_one();
    }

    bool get(T &result) {
        boost::unique_lock<boost::mutex> lock(mutex);
        cv.wait(lock, [this]()->bool {
            return !q.empty() || !on;
        });
        if (q.empty())
            return false;
        result = q.front();
        q.pop();
        return true;
    }

    void finish() {
        boost::unique_lock<boost::mutex> lock(mutex);
        on = false;
        cv.notify_all();
    }

    bool is_finished() {
        boost::unique_lock<boost::mutex> lock(mutex);
        return !on && q.empty();
    }

    bool empty() {
        boost::unique_lock<boost::mutex> lock (mutex);
        return q.empty();
    }
};


#endif // QUEUE_H
