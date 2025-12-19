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
// Includes
#include <algorithm>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <set>
#include <stack>
#include <thread>
#include <vector>
#include <turbo/times/time.h>

namespace tally {

    // Public types
    using timer_id = std::size_t;
    using handler_t = std::function<void(timer_id)>;
    static constexpr timer_id INVALID_TIMER_ID = std::numeric_limits<size_t>::max();
    // Private definitions. Do not rely on this namespace.
    namespace detail {

        // The event structure that holds the information about a timer.
        struct Event {
            timer_id id{0};
            turbo::Time start;
            turbo::Duration period;
            handler_t handler;
            bool valid{false};

            Event() = default;

            template<typename Func>
            Event(timer_id id, turbo::Time start, turbo::Duration period, Func &&handler)
                    : id(id), start(start), period(period), handler(std::forward<Func>(handler)), valid(true) {
            }

            Event(Event &&r) = default;

            Event &operator=(Event &&ev) = default;

            Event(const Event &r) = delete;

            Event &operator=(const Event &r) = delete;
        };

        // A time event structure that holds the next timeout and a reference to its
        // Event struct.
        struct Time_event {
            turbo::Time next;
            timer_id ref;
        };

        inline bool operator<(const Time_event &l, const Time_event &r) {
            return l.next < r.next;
        }

    } // end namespace detail

    class TaskTimer {
    public:
        TaskTimer() : m{}, cond{}, worker{}, events{}, time_events{}, free_ids{} {
            scoped_m lock(m);
            done = false;
            worker = std::thread([this] { run(); });
        }

        ~TaskTimer() {
            scoped_m lock(m);
            done = true;
            lock.unlock();
            cond.notify_all();
            worker.join();
            events.clear();
            time_events.clear();
            while (!free_ids.empty()) {
                free_ids.pop();
            }
        }

        /**
         * Add a new timer.
         *
         * \param when The time at which the handler is invoked.
         * \param handler The callable that is invoked when the timer fires.
         * \param period The periodicity at which the timer fires. Only used for periodic timers.
         */
        timer_id run_at(const turbo::Time &when, handler_t &&handler) {
            return add_impl(when,std::forward<handler_t>(handler));
        }

        timer_id run_every(const turbo::Time &when, handler_t &&handler, const turbo::Duration &d) {
            return add_impl(when,std::forward<handler_t>(handler), d);
        }

        timer_id run_every(const turbo::Duration &first_d, handler_t &&handler, const turbo::Duration &d) {
            return add_impl(turbo::Time::current_time() + first_d,std::forward<handler_t>(handler), d);
        }

        timer_id run_every(const uint64_t when_ms, handler_t &&handler, const uint64_t period_ms) {
            return run_every(turbo::Duration::microseconds(when_ms), std::move(handler), turbo::Duration::microseconds(period_ms));
        }

        timer_id run_after(const turbo::Duration &d, handler_t &&handler) {
            return add_impl(turbo::Time::current_time() + d,
                       std::move(handler));
        }
        timer_id run_after(const uint64_t when_ms, handler_t &&handler) {
            return run_after(turbo::Duration::microseconds(when_ms), std::move(handler));
        }

        /**
         * Removes the timer with the given id.
         */
        bool remove(timer_id id) {
            scoped_m lock(m);
            if (events.size() == 0 || events.size() <= id) {
                return false;
            }
            events[id].valid = false;
            events[id].handler = nullptr;
            auto it = std::find_if(time_events.begin(), time_events.end(),
                                   [&](const detail::Time_event &te) { return te.ref == id; });
            if (it != time_events.end()) {
                free_ids.push(it->ref);
                time_events.erase(it);
            }
            lock.unlock();
            cond.notify_all();
            return true;
        }

    private:

        timer_id add_impl(
                const turbo::Time &when, handler_t &&handler, const turbo::Duration &period = turbo::Duration()) {
            scoped_m lock(m);
            timer_id id = 0;
            // Add a new event. Prefer an existing and free id. If none is available, add
            // a new one.
            if (free_ids.empty()) {
                id = events.size();
                detail::Event e(id, when, period, std::move(handler));
                events.push_back(std::move(e));
            } else {
                id = free_ids.top();
                free_ids.pop();
                detail::Event e(id, when, period, std::move(handler));
                events[id] = std::move(e);
            }
            time_events.insert(detail::Time_event{when, id});
            lock.unlock();
            cond.notify_all();
            return id;
        }

        void run() {
            scoped_m lock(m);

            while (!done) {

                if (time_events.empty()) {
                    // Wait for work
                    cond.wait(lock);
                } else {
                    detail::Time_event te = *time_events.begin();
                    if (turbo::Time::current_time() >= te.next) {

                        // Remove time event
                        time_events.erase(time_events.begin());

                        // Invoke the handler
                        lock.unlock();
                        events[te.ref].handler(te.ref);
                        lock.lock();
                        static auto zero = turbo::Duration::zero();
                        if (events[te.ref].valid && events[te.ref].period > zero) {
                            // The event is valid and a periodic timer.
                            te.next += events[te.ref].period;
                            time_events.insert(te);
                        } else {
                            // The event is either no longer valid because it was removed in the
                            // callback, or it is a one-shot timer.
                            events[te.ref].valid = false;
                            events[te.ref].handler = nullptr;
                            free_ids.push(te.ref);
                        }
                    } else {
                        cond.wait_until(lock, turbo::Time::to_chrono(te.next));
                    }
                }
            }
        }
    private:
        using scoped_m = std::unique_lock<std::mutex>;

        // Thread and locking variables.
        std::mutex m;
        std::condition_variable cond;
        std::thread worker;

        // Use to terminate the timer thread.
        bool done = false;

        // The vector that holds all active events.
        std::vector<detail::Event> events;
        // Sorted queue that has the next timeout at its top.
        std::multiset<detail::Time_event> time_events;

        // A list of ids to be re-used. If possible, ids are used from this pool.
        std::stack<timer_id> free_ids;

    };

} // namespace tally

