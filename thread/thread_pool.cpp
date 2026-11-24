/* clang-format off */
/*****************************************************************************
  The MIT License

  Copyright © 2015 Pavel Karelin (hkarel), <hkarel@yandex.ru>

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*****************************************************************************/

#include "thread_pool.h"
#include "break_point.h"

namespace trd {

ThreadPool::~ThreadPool()
{
    if (!_stopped) stop();
}

void ThreadPool::start()
{
    _stopped = false;
}

void ThreadPool::stop()
{
    _stopped = true;
    for (Item* it : _threads)
        if (it->_sleeps.load(memory_order_relaxed))
        {
            lock_guard<mutex> locker {it->_sleepMutex}; (void) locker;
            it->_sleepCond.notify_all();
        }

    for (Item* item : _threads)
        while (!item->_finished.load(memory_order_relaxed))
        {
            static chrono::milliseconds timeout {5};
            this_thread::sleep_for(timeout);
        }

    for (Item* item : _threads)
        delete item;
    _threads.clear();
}

void ThreadPool::setTimeout(int timeout)
{
    if (_stopped.load(memory_order_relaxed))
        _timeout = timeout;
}

ThreadPool::Item* ThreadPool::run(Func runFunc)
{
    if (!runFunc)
        return nullptr;

    // Если пул остановлен - выполняем функцию в текущем потоке
    if (_stopped.load(memory_order_relaxed))
    {
        runFunc();
        return nullptr;
    }

    lock_guard<mutex> locker {_threadsLock}; (void) locker;

    Item* item = nullptr;
    for (Item* it : _threads)
    {
        if (it->_working.load(memory_order_relaxed))
            continue;

        if (it->_finished.load(memory_order_relaxed))
        {
            item = it;
            break;
        }

        if (it->_sleeps.load(memory_order_relaxed))
        {
            it->_func = std::move(runFunc);
            it->_working = true;
            { //Block for lock_guard
                lock_guard<mutex> locker {it->_sleepMutex}; (void) locker;
                it->_sleepCond.notify_all();
            }
            return it;
        }
    }

    if (item == nullptr)
    {
        item = new Item;
        _threads.push_back(item);
    }
    item->_pool = this;
    item->_timeout = chrono::seconds(_timeout);
    item->_working = true;
    item->_sleeps = false;
    item->_finished = false;
    item->_func = std::move(runFunc);
    item->_thread = thread(&Item::run, item);
    item->_thread.detach();
    return item;
}

void ThreadPool::Item::run()
{
    while (true)
    {
        if (_pool->_stopped.load(memory_order_relaxed))
            break;

        if (_func)
        {
            _func();
            _func = nullptr_t(); // Обнуляем рабочую функцию
        }

        // Тонкость: если вызывать _workCond.notify_all()  без блокирования
        // мьютекса _workMutex, то в функции ThreadPool::Item::join() будут
        // периодически  происходить не срабатывания _workCond.wait(),  что
        // фактически будет приводить к блокировке потока,  который  сделал
        // вызов ThreadPool::Item::join()
        {
            lock_guard<mutex> locker {_workMutex}; (void) locker;
            _working = false;
            _workCond.notify_all();
        }
        //asm volatile ("" ::: "memory");

        if (_pool->_stopped.load(memory_order_relaxed))
            break;

        _sleeps.store(true, memory_order_relaxed);
        cv_status stat;
        do {
            if (_working.load(memory_order_relaxed)
                || _pool->_stopped.load(memory_order_relaxed))
                break;

            unique_lock<mutex> locker {_sleepMutex};
            stat = _sleepCond.wait_for(locker, _timeout);

        } while (stat != cv_status::timeout);
        _sleeps.store(false, memory_order_relaxed);

        if (!_working.load(memory_order_relaxed)
            || _pool->_stopped.load(memory_order_relaxed))
            break;
    }
    _finished = true;
}

void ThreadPool::Item::join()
{
    if (_working)
    {
        unique_lock<mutex> locker {_workMutex}; (void) locker;
//        static chrono::microseconds timeout {5};
//        while (!_joinCond.wait_for(locker, timeout, [this]{
//            //return !this->_working.load(memory_order_acquire);
//            return !this->_working;
//        })) ;
        _workCond.wait(locker, [this]{return !this->_working;});
    }
}

ThreadPool& threadPool()
{
    return safe::singleton<ThreadPool>();
}

} // namespace trd
