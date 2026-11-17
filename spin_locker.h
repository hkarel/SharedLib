/* clang-format off */
/*****************************************************************************
  The MIT License

  Copyright © 2013 Pavel Karelin (hkarel), <hkarel@yandex.ru>

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

#pragma once

#include <atomic>

#ifdef SPIN_LOCKER_REV2
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#endif
#endif // SPIN_LOCKER_REV2

/**
  Простейший механизм для автоматической блокировки/разблокировки
  атомарного локкера
*/
struct SpinLocker
{
    explicit SpinLocker(std::atomic_flag& locker) noexcept : locker(locker) {
        lock();
    }
    ~SpinLocker() noexcept {
        unlock();
    }
    void lock() {
#ifdef SPIN_LOCKER_REV2
        while (locker.test_and_set(std::memory_order_acquire)) {
        #if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
            _mm_pause();
        #elif defined(__aarch64__) || defined(_M_ARM64)
            asm volatile("yield" ::: "memory");
        #endif
        }
#else
        while (locker.test_and_set(std::memory_order_acquire)) {}
#endif // SPIN_LOCKER_REV2
    }
    void unlock() {
        locker.clear(std::memory_order_release);
    }
    std::atomic_flag& locker;

    SpinLocker() = delete;
    SpinLocker(SpinLocker&&) = delete;
    SpinLocker(const SpinLocker&) = delete;
    SpinLocker& operator= (SpinLocker&&) = delete;
    SpinLocker& operator= (const SpinLocker&) = delete;
};
