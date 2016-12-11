#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <list>
#include <limits>
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <stdio.h>

template<class T> class Singleton
{
//Singleton
//http://www.yolinux.com/TUTORIALS/C++Singleton.html

public:
    static T* inst()
    {
        if (!m_pInstance)
        {
            m_pInstance = new T;
        }
        return m_pInstance;
    }

private:
    Singleton(){};
    Singleton(const Singleton&){};
    Singleton& operator=(const Singleton&){};

    static T* m_pInstance;
};

template<class T> T* Singleton<T>::m_pInstance = NULL;

class HPTimer
{
private:
    LARGE_INTEGER t0;
    LARGE_INTEGER t1;
    LARGE_INTEGER f;

    double et, last_dt;
    bool na;

public:
    HPTimer::HPTimer()
    {
        QueryPerformanceFrequency(&f);
        na = f.QuadPart == 0;
        reset();
    }

    void HPTimer::reset()
    {
        QueryPerformanceCounter(&t0);
        last_dt = 0.0;
        et = 0.0;
    }

    double HPTimer::time()
    {
        double dt;

        if (na) return 0.0;

        QueryPerformanceCounter(&t1);
        dt = double(t1.QuadPart - t0.QuadPart);
        if (dt<0.0)
        {
            et = et + last_dt / f.QuadPart;
            t0 = t1;
            last_dt = 0.0;
            return et;
        }
        else
        {
            last_dt = dt;
            return et + dt / f.QuadPart;
        }
    }
};

template <class T> class SP
{
private:
    T *m_data;
    int *m_count;

public:
    SP() : m_data(NULL), m_count(NULL)
    {
        m_count = new int(1);
        (*m_count)++;
    }

    SP(T *ptr) : m_data(ptr), m_count(NULL)
    {
        m_count = new int(1);
        (*m_count)++;
    }

    SP(const SP<T> &sp) : m_count(sp.m_count)
    {
        (*m_count)++;
    }

    ~SP()
    {
        if (--(*m_count) == 0)
        {
            delete m_data;
            delete m_count;
        }
    }

    T& operator*()
    {
        return *m_data;
    }

    const T& operator*() const 
    {
        return *m_data;
    }

    T* operator->()
    {
        return m_data;
    }

    const T* operator->() const
    {
        return m_data;
    }

    T* operator &()
    {
        return m_data;
    }

    SP<T> &operator=(const SP<T> &sp)
    {
        if (this != &sp)
        {
            if (--(*m_count) == 0)
            {
                delete m_data;
                delete m_count;
            }

            m_data = sp.m_data;
            m_count = sp.m_count;
            (*m_count)++;
        }

        return *this;
    }
};

template <class T>
void timeout(const T& callback, unsigned int timeInMs)
{
    concurrency::task_completion_event<void> tce;
    auto call = new concurrency::call<int>(
        [callback, tce](int)
        {
            callback();
            tce.set();
        });

    auto timer = new concurrency::timer<int>(timeInMs, 0, call, true);
    //concurrency::task<void> event_set(tce);
    //event_set.then([timer, call]()
    //{
    //    delete call;
    //    delete timer;
    //});

    timer->start();
}


#endif

