// Copyright (C) 2024 Kumo inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#pragma once

#include <turbo/base/macros.h>
#include <turbo/log/logging.h>
#include <mutex>                           // std::lock_guard


namespace tally {
    namespace detail {
        template<typename T>
        std::lock_guard<typename std::remove_reference<T>::type> get_lock_guard();
    }  // namespace detail
}  // namespace tally

namespace std {

#if defined(__linux__) || defined(__MACH__)

    template<>
    class lock_guard<pthread_mutex_t> {
    public:
        explicit lock_guard(pthread_mutex_t &mutex) : _pmutex(&mutex) {
            pthread_mutex_lock(_pmutex);
        }

        ~lock_guard() {
            pthread_mutex_unlock(_pmutex);
        }

        lock_guard(const lock_guard &) = delete;

        lock_guard &operator=(const lock_guard &) = delete;

    private:
        pthread_mutex_t *_pmutex;
    };

    template<>
    class lock_guard<pthread_spinlock_t> {
    public:
        explicit lock_guard(pthread_spinlock_t &spin) : _pspin(&spin) {
            pthread_spin_lock(_pspin);
        }

        ~lock_guard() {
            pthread_spin_unlock(_pspin);
        }

        lock_guard(const lock_guard &) = delete;

        lock_guard &operator=(const lock_guard &) = delete;

    private:
        pthread_spinlock_t *_pspin;
    };

    template<>
    class unique_lock<pthread_mutex_t> {
    public:
        unique_lock(const unique_lock &) = delete;

        unique_lock &operator=(const unique_lock &) = delete;

        typedef pthread_mutex_t mutex_type;

        unique_lock() : _mutex(NULL), _owns_lock(false) {}

        explicit unique_lock(mutex_type &mutex)
                : _mutex(&mutex), _owns_lock(true) {
            pthread_mutex_lock(_mutex);
        }

        unique_lock(mutex_type &mutex, defer_lock_t)
                : _mutex(&mutex), _owns_lock(false) {}

        unique_lock(mutex_type &mutex, try_to_lock_t)
                : _mutex(&mutex), _owns_lock(pthread_mutex_trylock(&mutex) == 0) {}

        unique_lock(mutex_type &mutex, adopt_lock_t)
                : _mutex(&mutex), _owns_lock(true) {}

        ~unique_lock() {
            if (_owns_lock) {
                pthread_mutex_unlock(_mutex);
            }
        }

        void lock() {
            if (_owns_lock) {
                KCHECK(false) << "Detected deadlock issue";
                return;
            }
            _owns_lock = true;
            pthread_mutex_lock(_mutex);
        }

        bool try_lock() {
            if (_owns_lock) {
                KCHECK(false) << "Detected deadlock issue";
                return false;
            }
            _owns_lock = !pthread_mutex_trylock(_mutex);
            return _owns_lock;
        }

        void unlock() {
            if (!_owns_lock) {
                KCHECK(false) << "Invalid operation";
                return;
            }
            pthread_mutex_unlock(_mutex);
            _owns_lock = false;
        }

        void swap(unique_lock &rhs) {
            std::swap(_mutex, rhs._mutex);
            std::swap(_owns_lock, rhs._owns_lock);
        }

        mutex_type *release() {
            mutex_type *saved_mutex = _mutex;
            _mutex = NULL;
            _owns_lock = false;
            return saved_mutex;
        }

        mutex_type *mutex() { return _mutex; }

        bool owns_lock() const { return _owns_lock; }

        operator bool() const { return owns_lock(); }

    private:
        mutex_type *_mutex;
        bool _owns_lock;
    };

    template<>
    class unique_lock<pthread_spinlock_t> {
    public:
        typedef pthread_spinlock_t mutex_type;

        unique_lock() : _mutex(nullptr), _owns_lock(false) {}

        explicit unique_lock(mutex_type &mutex)
                : _mutex(&mutex), _owns_lock(true) {
            pthread_spin_lock(_mutex);
        }

        ~unique_lock() {
            if (_owns_lock) {
                pthread_spin_unlock(_mutex);
            }
        }

        unique_lock(mutex_type &mutex, defer_lock_t)
                : _mutex(&mutex), _owns_lock(false) {}

        unique_lock(mutex_type &mutex, try_to_lock_t)
                : _mutex(&mutex), _owns_lock(pthread_spin_trylock(&mutex) == 0) {}

        unique_lock(mutex_type &mutex, adopt_lock_t)
                : _mutex(&mutex), _owns_lock(true) {}

        void lock() {
            if (_owns_lock) {
                KCHECK(false) << "Detected deadlock issue";
                return;
            }
            _owns_lock = true;
            pthread_spin_lock(_mutex);
        }

        bool try_lock() {
            if (_owns_lock) {
                KCHECK(false) << "Detected deadlock issue";
                return false;
            }
            _owns_lock = !pthread_spin_trylock(_mutex);
            return _owns_lock;
        }

        void unlock() {
            if (!_owns_lock) {
                KCHECK(false) << "Invalid operation";
                return;
            }
            pthread_spin_unlock(_mutex);
            _owns_lock = false;
        }

        void swap(unique_lock &rhs) {
            std::swap(_mutex, rhs._mutex);
            std::swap(_owns_lock, rhs._owns_lock);
        }

        mutex_type *release() {
            mutex_type *saved_mutex = _mutex;
            _mutex = NULL;
            _owns_lock = false;
            return saved_mutex;
        }

        mutex_type *mutex() { return _mutex; }

        bool owns_lock() const { return _owns_lock; }

        operator bool() const { return owns_lock(); }

        unique_lock(const unique_lock &) = delete;

        unique_lock &operator=(const unique_lock &) = delete;

    private:
        mutex_type *_mutex;
        bool _owns_lock;
    };

#endif  // defined(__linux__) || defined(__MACH__)

}  // namespace std

namespace tally {

    // Lock both lck1 and lck2 without the dead lock issue
    template<typename Mutex1, typename Mutex2>
    void double_lock(std::unique_lock<Mutex1> &lck1, std::unique_lock<Mutex2> &lck2) {
        DKCHECK(!lck1.owns_lock());
        DKCHECK(!lck2.owns_lock());
        volatile void *const ptr1 = lck1.mutex();
        volatile void *const ptr2 = lck2.mutex();
        DKCHECK_NE(ptr1, ptr2);
        if (ptr1 < ptr2) {
            lck1.lock();
            lck2.lock();
        } else {
            lck2.lock();
            lck1.lock();
        }
    }

};
