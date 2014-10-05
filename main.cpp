#define BOOST_TEST_MODULE stringtest
#include <iostream>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <boost/test/included/unit_test.hpp>

#include "threadpool.h"

using namespace std;

struct matrix {
    vector<vector<int>> data;
    int size;
    matrix(int _size):size(_size) {
        data.resize(size);
        for (int i = 0; i < size; i++) {
            data[i].resize(size);
        }
    }
    void operator = (matrix a) {
        size = a.size;
        data = a.data;
    }
};

void write(matrix const &m) {
    for (int i = 0; i < m.size; i++) {
        for (int j = 0; j < m.size; j++) {
            cout << m.data[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void generate(matrix &m) {
    srand(time(0));
    for (int i = 0; i < m.size; i++)
        for (int j = 0; j < m.size; j++)
            m.data[i][j] = rand() % 10;
}

matrix multiply(matrix &a, matrix &b) {
    if (a.size != b.size)
        throw "плохо";
    matrix c(a.size);
    for (int i = 0; i < a.size; i++) {
        for (int j = 0; j < b.size; j++) {
            for (int k = 0; k < c.size; k++) {
                c.data[i][j] += a.data[i][k] * b.data[k][j];
            }
        }
    }
    return c;
}

boost::mutex mut;

void sum(matrix const &a, matrix const &b, shared_ptr<matrix> c, int i, int j) {
    for (int k = 0; k < a.size; k++) {
        c->data[i][j] += a.data[i][k] * b.data[k][j];
    }
}

matrix multithr_muktiply(matrix const &a, matrix const &b) {
    if (a.size != b.size)
        throw "плохо";
    shared_ptr<matrix> pointer (new matrix(a.size));
    my::threadpool pool;
    int count = a.size * a.size;
    for (int i = 0; i < a.size; i++) {
        for (int j = 0; j < b.size; j++) {
            pool.add<void>(count--, sum, a, b, pointer, i, j);
        }
    }
    pool.stop();
    cout << endl;
    return *pointer;
}

BOOST_AUTO_TEST_SUITE (stringtest)

BOOST_AUTO_TEST_CASE (test1)
{
    matrix a(100), b(100);
    generate(a);
    generate(b);
    clock_t t1 = clock();
    matrix c = multiply(a, b);
    t1 = clock() - t1;
    clock_t t2 = clock();
    matrix d = multithr_muktiply(a, b);
    t2 = clock() - t2;
    for (int i = 0; i < c.size; i++) {
        for (int j = 0; j < c.size; j++) {
            BOOST_CHECK(c.data[i][j] == d.data[i][j]);
        }
    }
    cout << t2 - t1 << endl;
}

BOOST_AUTO_TEST_CASE (testGet) {
    my::priority_queue<int, int> q;
    const int n = 10;
    for (int t = 0; t < 10000; ++t) {
        if (t % 100 == 0)
            std::cout << t << std::endl;
        std::vector<std::shared_ptr<boost::thread>> threads;
        for (int i = 0; i < n; ++i) {
            q.put(i, i);
        }
        std::vector<int> result(n);
        for (int i = 0; i < n; ++i) {
            auto th = std::make_shared<boost::thread> (boost::thread([&q, i, &result]{q.get(result[i]);}));
            threads.push_back(th);
        }
        for (int i = 0; i < n; ++i) {
            threads[i]->join();
        }
    }
}

BOOST_AUTO_TEST_CASE (testPut) {
    const int n = 100;
    const int t = 10000;
    my::priority_queue<int, int> q;
    for (int i = 0; i < t; ++i) {
        if (i % 100 == 0)
            std::cout << i << std::endl;
        std::vector<boost::thread> threads;
        for (int j = 0; j < n; ++j) {
            threads.push_back(boost::thread([&q, j]{q.put(j, j);}));
        }
        for (int j = 0; j < n; ++j) {
            threads[j].join();
        }
        std::vector<int> result(n);
        for (int j = 0; j < n; ++j) {
            q.get(result[j]);
        }
        for (int j = 0; j < n - 1; ++j) {
            BOOST_CHECK(result[j] >= result[j + 1]);
        }
    }
}

BOOST_AUTO_TEST_CASE (testPutGet) {
    const int n = 10;
    const int t = 10000;
    for (int i = 0; i < t; ++i) {
        my::priority_queue<int, int> q;
        if (i % 100 == 0)
            std::cout << i << std::endl;
        std::vector<boost::thread> threads;
        for (int j = 0; j < n; ++j) {
                threads.push_back(boost::thread([&q, i, j]{q.put(i + 2 * j, i + 2 * j);}));
                threads.push_back(boost::thread([&q, i, j]{q.put(i * j - j, i * j - j);}));
                threads.push_back(boost::thread([&q]{int a; q.get(a);}));
        }
        for (int j = 0; j < n * 3; ++j) {
            threads[j].join();
        }
        std::vector<int> result(n);
        for (int j = 0; j < n; ++j) {
            q.get(result[j]);
        }
        for (int j = 0; j < n - 1; ++j) {
            BOOST_CHECK(result[j] >= result[j + 1]);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END( )
