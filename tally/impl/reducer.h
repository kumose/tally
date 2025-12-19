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

#include <limits>                                 // std::numeric_limits
#include <turbo/log/logging.h>                         // KLOG()
#include <tally/utility/type_traits.h>                     // tally::add_cr_non_integral
#include <turbo/base/class_name.h>                      // class_name_str
#include <tally/variable.h>                        // Variable
#include <tally/impl/combiner.h>                 // detail::AgentCombiner
#include <tally/impl/sampler.h>                  // ReducerSampler
#include <tally/impl/series.h>
#include <tally/window.h>
#include <tally/config.h>
#include <turbo/log/logging.h>
#include <tally/stats_reporter.h>

namespace tally {

    // Reduce multiple values into one with `Op': e1 Op e2 Op e3 ...
    // `Op' shall satisfy:
    //   - associative:     a Op (b Op c) == (a Op b) Op c
    //   - commutative:     a Op b == b Op a;
    //   - no side effects: a Op b never changes if a and b are fixed.
    // otherwise the result is undefined.
    //
    // For performance issues, we don't let Op return value, instead it shall
    // set the result to the first parameter in-place. Namely to add two values,
    // "+=" should be implemented rather than "+".
    //
    // Reducer works for non-primitive T which satisfies:
    //   - T() should be the identity of Op.
    //   - stream << v should compile and put description of v into the stream
    // Example:
    // class MyType {
    // friend std::ostream& operator<<(std::ostream& os, const MyType&);
    // public:
    //     MyType() : _x(0) {}
    //     explicit MyType(int x) : _x(x) {}
    //     void operator+=(const MyType& rhs) const {
    //         _x += rhs._x;
    //     }
    // private:
    //     int _x;
    // };
    // std::ostream& operator<<(std::ostream& os, const MyType& value) {
    //     return os << "MyType{" << value._x << "}";
    // }
    // tally::Adder<MyType> my_type_sum;
    // my_type_sum << MyType(1) << MyType(2) << MyType(3);
    // KLOG(INFO) << my_type_sum;  // "MyType{6}"

    template<typename T, typename Op, typename InvOp = detail::VoidOp>
    class Reducer : public Variable {
    public:
        typedef typename detail::AgentCombiner<T, T, Op> combiner_type;
        typedef typename combiner_type::Agent agent_type;
        typedef detail::ReducerSampler<Reducer, T, Op, InvOp> sampler_type;

        class SeriesSampler : public detail::Sampler {
        public:
            SeriesSampler(Reducer *owner, const Op &op)
                    : _owner(owner), _series(op) {}

            ~SeriesSampler() = default;

            void take_sample() override { _series.append(_owner->get_value()); }

            void describe(std::ostream &os) { _series.describe(os, nullptr); }

        private:
            Reducer *_owner;
            detail::Series<T, Op> _series;
        };

    public:
        // The `identify' must satisfy: identity Op a == a
        Reducer(VariableAttr attr, typename add_cr_non_integral<T>::type identity = T(),
                const Op &op = Op(),
                const InvOp &inv_op = InvOp())
                : Variable(attr), _combiner(identity, identity, op), _sampler(nullptr), _series_sampler(nullptr), _inv_op(inv_op) {
        }

        ~Reducer() {
            // Calling hide() manually is a MUST required by Variable.
            hide();
            if (_sampler) {
                _sampler->destroy();
                _sampler = nullptr;
            }
            if (_series_sampler) {
                _series_sampler->destroy();
                _series_sampler = nullptr;
            }
        }

        // Add a value.
        // Returns self reference for chaining.
        Reducer &operator<<(typename add_cr_non_integral<T>::type value);

        // Get reduced value.
        // Notice that this function walks through threads that ever add values
        // into this reducer. You should avoid calling it frequently.
        T get_value() const {
            KCHECK(!(std::is_same<InvOp, detail::VoidOp>::value) || _sampler == nullptr)
                            << "You should not call Reducer<" << turbo::class_name_str<T>()
                            << ", " << turbo::class_name_str<Op>() << ">::get_value() when a"
                            << " Window<> is used because the operator does not have inverse.";
            return _combiner.combine_agents();
        }

        void get_value(std::any *value) const override {
            *value = get_value();
        }

        // Reset the reduced value to T().
        // Returns the reduced value before reset.
        T reset() { return _combiner.reset_all_agents(); }

        void describe(std::ostream &os, bool quote_string) const override {
            if (std::is_same<T, std::string>::value && quote_string) {
                os << '"' << get_value() << '"';
            } else {
                os << get_value();
            }
        }

        // True if this reducer is constructed successfully.
        bool valid() const { return _combiner.valid(); }

        // Get instance of Op.
        const Op &op() const { return _combiner.op(); }

        const InvOp &inv_op() const { return _inv_op; }

        sampler_type *get_sampler() {
            if (nullptr == _sampler) {
                _sampler = new sampler_type(this);
                _sampler->schedule();
            }
            return _sampler;
        }

        turbo::Status describe_series(std::ostream& os, const SeriesOptions& options) const override {
            if (_series_sampler == nullptr) {
                return turbo::unavailable_error("");
            }
            if (!options.test_only) {
                _series_sampler->describe(os);
            }
            return turbo::OkStatus();
        }

    protected:
        turbo::Status
        expose_impl(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope) override {
            auto rc = Variable::expose_impl(name, help, scope);
            if (rc.ok() &&
                _series_sampler == nullptr &&
                !std::is_same<InvOp, detail::VoidOp>::value &&
                !std::is_same<T, std::string>::value &&
                turbo::get_flag(FLAGS_tally_save_series)) {
                _series_sampler = new SeriesSampler(this, _combiner.op());
                _series_sampler->schedule();
            }
            return rc;
        }

    private:
        combiner_type _combiner;
        sampler_type *_sampler;
        SeriesSampler *_series_sampler;
        InvOp _inv_op;
    };

    template<typename T, typename Op, typename InvOp>
    inline Reducer<T, Op, InvOp> &Reducer<T, Op, InvOp>::operator<<(
            typename add_cr_non_integral<T>::type value) {
        // It's wait-free for most time
        agent_type *agent = _combiner.get_or_create_tls_agent();
        if (__builtin_expect(!agent, 0)) {
            KLOG(FATAL) << "Fail to create agent";
            return *this;
        }
        agent->element.modify(_combiner.op(), value);
        return *this;
    }

    // =================== Common reducers ===================

    // tally::Adder<int> sum;
    // sum << 1 << 2 << 3 << 4;
    // KLOG(INFO) << sum.get_value(); // 10
    // Commonly used functors
    namespace detail {
        template<typename Tp>
        struct AddTo {
            void operator()(Tp &lhs,
                            typename add_cr_non_integral<Tp>::type rhs) const { lhs += rhs; }
        };

        template<typename Tp>
        struct MinusFrom {
            void operator()(Tp &lhs,
                            typename add_cr_non_integral<Tp>::type rhs) const { lhs -= rhs; }
        };
    }
    // tally::Maxer<int> max_value;
    // max_value << 1 << 2 << 3 << 4;
    // KLOG(INFO) << max_value.get_value(); // 4
    namespace detail {
        template<typename Tp>
        struct MaxTo {
            void operator()(Tp &lhs,
                            typename add_cr_non_integral<Tp>::type rhs) const {
                // Use operator< as well.
                if (lhs < rhs) {
                    lhs = rhs;
                }
            }
        };

        class LatencyRecorderBase;
    }

    // tally::Miner<int> min_value;
    // min_value << 1 << 2 << 3 << 4;
    // KLOG(INFO) << min_value.get_value(); // 1
    namespace detail {

        template<typename Tp>
        struct MinTo {
            void operator()(Tp &lhs,
                            typename add_cr_non_integral<Tp>::type rhs) const {
                if (rhs < lhs) {
                    lhs = rhs;
                }
            }
        };

    }  // namespace detail

}  // namespace tally
