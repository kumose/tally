// Copyright (C) Kumo inc. and its affiliates.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <cstdint>
#include <tally/impl/reducer.h>
#include <tally/scope.h>

namespace tally {

    template<typename T, typename Enable = void>
    class Counter;
    template<typename T>
    class Counter<T, typename std::enable_if<std::is_integral_v<T> || std::is_floating_point_v<T>>::type> :  public Reducer<T, detail::AddTo<T>, detail::MinusFrom<T> > {
    public:
        typedef Reducer<T, detail::AddTo<T>, detail::MinusFrom<T> > Base;
        typedef T value_type;
        typedef typename Base::sampler_type sampler_type;
    public:
        Counter() : Base(VariableAttr::counter_attr()) {}

        explicit Counter(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope = ScopeInstance::instance()->get_default().get()) : Base(VariableAttr::counter_attr()) {
            auto rs = this->expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL,turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose Counter: "<<name<<"fail reason: "<<rs.to_string();
                KLOG(WARNING)<<"expose Counter: "<<name<<"fail reason: "<<rs.to_string();
            }
        }

        // Increments the counter by 1.
        Counter &increment() {
            return increment(1);
        }

        // Increments the counter by `value`.
        Counter&increment(double value) {
            *this<<value;
            return *this;
        }

        MetricSample get_metric(const turbo::Time &stamp) const override {
            return {VariableType::counter_type(), static_cast<double>(Base::get_value()), stamp};
        }

        ~Counter() { Variable::hide(); }
    };

    //using Counter = CounterBase<double>;
    //using IntCount = CounterBase<int64_t>;

}  // namespace tally
