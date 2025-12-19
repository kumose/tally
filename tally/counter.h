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
