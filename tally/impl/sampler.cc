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

#include <turbo/memory/leaky_singleton.h>
#include <tally/impl/reducer.h>
#include <tally/impl/sampler.h>
#include <tally/passive_status.h>
#include <tally/window.h>
#include <tally/config.h>

namespace tally::detail {

    const int WARN_NOSLEEP_THRESHOLD = 2;

    // Combine two circular linked list into one.
    struct CombineSampler {
        void operator()(Sampler* & s1, Sampler* s2) const {
            if (s2 == nullptr) {
                return;
            }
            if (s1 == nullptr) {
                s1 = s2;
                return;
            }
            s1->InsertBeforeAsList(s2);
        }
    };

    // True iff pthread_atfork was called. The callback to atfork works for child
    // of child as well, no need to register in the child again.
    static bool registered_atfork = false;

    // Call take_sample() of all scheduled samplers.
    // This can be done with regular timer thread, but it's way too slow(global
    // contention + log(N) heap manipulations). We need it to be super fast so that
    // creation overhead of Window<> is negliable.
    // The trick is to use Reducer<Sampler*, CombineSampler>. Each Sampler is
    // doubly linked, thus we can reduce multiple Samplers into one cicurlarly
    // doubly linked list, and multiple lists into larger lists. We create a
    // dedicated thread to periodically get_value() which is just the combined
    // list of Samplers. Waking through the list and call take_sample().
    // If a Sampler needs to be deleted, we just mark it as unused and the
    // deletion is taken place in the thread as well.
    class SamplerCollector : public Reducer<Sampler*, CombineSampler> {
    public:
        SamplerCollector()
            : Reducer<Sampler*, CombineSampler>(VariableAttr::sampler_attr()), _created(false)
            , _stop(false)
            , _cumulated_time_us(0) {
            create_sampling_thread();
        }
        ~SamplerCollector() {
            if (_created) {
                _stop = true;
                pthread_join(_tid, nullptr);
                _created = false;
            }
        }

    private:
        // Support for fork:
        // * The singleton can be null before forking, the child callback will not
        //   be registered.
        // * If the singleton is not null before forking, the child callback will
        //   be registered and the sampling thread will be re-created.
        // * A forked program can be forked again.

        static void child_callback_atfork() {
            turbo::get_leaky_singleton<SamplerCollector>()->after_forked_as_child();
        }

        void create_sampling_thread() {
            const int rc = pthread_create(&_tid, nullptr, sampling_thread, this);
            if (rc != 0) {
                KLOG(FATAL) << "Fail to create sampling_thread";
            } else {
                _created = true;
                if (!registered_atfork) {
                    registered_atfork = true;
                    pthread_atfork(nullptr, nullptr, child_callback_atfork);
                }
            }
        }

        void after_forked_as_child() {
            _created = false;
            create_sampling_thread();
        }

        void run();

        static void* sampling_thread(void* arg) {
            //tally::PlatformThread::SetName("tally_sampler");
            static_cast<SamplerCollector*>(arg)->run();
            return nullptr;
        }

        static double get_cumulated_time(void* arg) {
            return static_cast<SamplerCollector*>(arg)->_cumulated_time_us / 1000.0 / 1000.0;
        }

    private:
        bool _created;
        bool _stop;
        int64_t _cumulated_time_us;
        pthread_t _tid;
    };


    void SamplerCollector::run() {
        ::usleep(turbo::get_flag(FLAGS_tally_sampler_thread_start_delay_us));

        turbo::LinkNode<Sampler> root;
        int consecutive_nosleep = 0;
        while (!_stop) {
            int64_t abstime = turbo::Time::current_microseconds();
            Sampler* s = this->reset();
            if (s) {
                s->InsertBeforeAsList(&root);
            }
            for (turbo::LinkNode<Sampler>* p = root.next(); p != &root;) {
                // We may remove p from the list, save next first.
                turbo::LinkNode<Sampler>* saved_next = p->next();
                Sampler* s = p->value();
                s->_mutex.lock();
                if (!s->_used) {
                    s->_mutex.unlock();
                    p->RemoveFromList();
                    delete s;
                } else {
                    s->take_sample();
                    s->_mutex.unlock();
                }
                p = saved_next;
            }
            bool slept = false;
            int64_t now = turbo::Time::current_microseconds();
            _cumulated_time_us += now - abstime;
            abstime += 1000000L;
            while (abstime > now) {
                ::usleep(abstime - now);
                slept = true;
                now = turbo::Time::current_microseconds();
            }
            if (slept) {
                consecutive_nosleep = 0;
            } else {
                if (++consecutive_nosleep >= WARN_NOSLEEP_THRESHOLD) {
                    consecutive_nosleep = 0;
                    KLOG(WARNING) << "tally is busy at sampling for "
                                 << WARN_NOSLEEP_THRESHOLD << " seconds!";
                }
            }
        }
    }

    Sampler::Sampler() : _used(true) {}

    Sampler::~Sampler() {}

    void Sampler::schedule() {
        // since the SamplerCollector is initialized before the program starts
        // flags will not take effect if used in the SamplerCollector constructor
        if (turbo::get_flag(FLAGS_tally_enable_sampling)) {
            *turbo::get_leaky_singleton<SamplerCollector>() << this;
        }
    }

    void Sampler::destroy() {
        _mutex.lock();
        _used = false;
        _mutex.unlock();
    }

}  // namespace tally::detail
