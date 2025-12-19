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

#include <string>                       // std::string
#include <atomic>
#include <mutex>
#include <tally/utility/type_traits.h>
#include <tally/impl/is_atomical.h>
#include <tally/variable.h>
#include <tally/config.h>
#include <tally/impl/reducer.h>

namespace tally {

    // Display a rarely or periodically updated value.
    // Usage:
    //   tally::Status<int> foo_count1(17);
    //   foo_count1.expose("my_value");
    //
    //   tally::Status<int> foo_count2;
    //   foo_count2.set_value(17);
    //
    //   tally::Status<int> foo_count3("my_value", 17);
    template <typename T, typename Enabler = void>
    class Status;

    template <typename T>
    class Status<T, typename std::enable_if<!detail::is_atomical<T>::value>::type> : public Variable {
    public:
        Status() : Variable(VariableAttr::status_attr()){}
        Status(const T& value) : Variable(VariableAttr::status_attr()),_value(value) {}
        Status(std::string_view name, std::string_view help, const T& value, Scope* scope = ScopeInstance::instance()->get_default().get()) : Variable(VariableAttr::status_attr()), _value(value) {
            auto rs = this->expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose Status failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose Status failed: "<<name<<"to scope"<<scope->id();
            }
        }
        // Calling hide() manually is a MUST required by Variable.
        ~Status() { hide(); }

        void describe(std::ostream& os, bool /*quote_string*/) const override {
            os << get_value();
        }

        void get_value(std::any* value) const override {
            std::unique_lock guard(_lock);
            *value = _value;
        }

        T get_value() const {
            std::unique_lock guard(_lock);
            const T res = _value;
            return res;
        }

        void set_value(const T& value) {
            std::unique_lock guard(_lock);
            _value = value;
        }

    private:
        T _value;
        // We use lock rather than std::atomic for generic values because
        // std::atomic requires the type to be memcpy-able (POD basically)
        mutable std::mutex _lock;
    };

    // Specialize for std::string, adding a printf-style set_value().
    template <>
    class Status<std::string, void> : public Variable {
    public:
        Status(): Variable(VariableAttr::status_attr()) {}

        Status(std::string_view name, std::string_view help, std::string_view sv, Scope* scope = ScopeInstance::instance()->get_default().get()) : Variable(VariableAttr::status_attr()) {
            _value = sv;
            auto rs = expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose Status failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose Status failed: "<<name<<"to scope"<<scope->id();
            }
        }

        template <typename... Args>
        Status(std::string_view name, std::string_view help, turbo::Nonnull<Scope*> scope, const turbo::FormatSpec<Args...>& fmt, const Args&... args) : Variable(VariableAttr::status_attr()) {
            _value = turbo::str_format(fmt, args...);
            auto rs = expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose Status failed: "<<name<<"to scope"<<scope->id();
            }
        }
        template <typename... Args>
        Status(std::string_view name, std::string_view help, const turbo::FormatSpec<Args...>& fmt, const Args&... args) : Variable(VariableAttr::status_attr()) {
            _value = turbo::str_format(fmt, args...);
            auto scope = ScopeInstance::instance()->get_default().get();
            auto rs = expose(name, help, ScopeInstance::instance()->get_default().get());
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose Status failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose Status failed: "<<name<<"to scope"<<scope->id();
            }
        }

        ~Status() { hide(); }

        void describe(std::ostream& os, bool quote_string) const override {
            if (quote_string) {
                os << '"' << get_value() << '"';
            } else {
                os << get_value();
            }
        }

        std::string get_value() const {
            std::unique_lock guard(_lock);
            return _value;
        }

        void get_value(std::any* value) const override {
            *value = get_value();
        }

        template <typename... Args>
        void set_value(const turbo::FormatSpec<Args...>& fmt, const Args&... args) {

            auto v = turbo::str_format(fmt, args...);
            std::unique_lock guard(_lock);
            _value = std::move(v);
        }

        void set_value(const std::string& s) {
            std::unique_lock guard(_lock);
            _value = s;
        }

    private:
        std::string _value;
        mutable std::mutex _lock;
    };

    template<typename T, typename Enabler = void>
    class MaxerStatus;

    /// is_atomical<T> should use gauge.
    template<typename T>
    class MaxerStatus<T, typename std::enable_if<!detail::is_atomical<T>::value>::type> : public Reducer<T, detail::MaxTo<T> > {
    public:
        typedef Reducer<T, detail::MaxTo<T> > Base;
        typedef T value_type;
        typedef typename Base::sampler_type sampler_type;
    public:
        MaxerStatus() : Base(VariableAttr::status_attr(), std::numeric_limits<T>::min()) {}

        explicit MaxerStatus(T v) : Base(VariableAttr::status_attr(), v) {}

        explicit MaxerStatus(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope = ScopeInstance::instance()->get_default().get())
                : Base(VariableAttr::status_attr(),std::numeric_limits<T>::min()) {
            auto rs = this->expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose MaxerGauge failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose MaxerGauge failed: "<<name<<"to scope"<<scope->id();
            }
        }
        void update(T v) {
            *this<<v;
        }

        ~MaxerStatus() { Variable::hide(); }
    };

    template<typename T, typename Enabler = void>
    class MinerStatus;

    template<typename T>
    class MinerStatus<T, typename std::enable_if<!detail::is_atomical<T>::value>::type> : public Reducer<T, detail::MinTo<T> > {
    public:
        typedef Reducer<T, detail::MinTo<T> > Base;
        typedef T value_type;
        typedef typename Base::sampler_type sampler_type;
    public:
        MinerStatus() : Base(VariableAttr::status_attr(), std::numeric_limits<T>::max()) {}

        explicit MinerStatus(T v) : Base(VariableAttr::status_attr(), v) {}

        explicit MinerStatus(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope = ScopeInstance::instance()->get_default().get())
                : Base(VariableAttr::status_attr(),std::numeric_limits<T>::max()) {
            auto rs = this->expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose MinerGauge failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose MinerGauge failed: "<<name<<"to scope"<<scope->id();
            }
        }

        void update(T v) {
            *this<<v;
        }

        ~MinerStatus() { Variable::hide(); }
    };

    template<typename T, typename Enabler = void>
    class AdderStatus;
    template<typename T>
    class AdderStatus<T, typename std::enable_if<!detail::is_atomical<T>::value>::type> : public Reducer<T, detail::AddTo<T>, detail::MinusFrom<T> > {
    public:
        typedef Reducer<T, detail::AddTo<T>, detail::MinusFrom<T> > Base;
        typedef T value_type;
        typedef typename Base::sampler_type sampler_type;
    public:
        AdderStatus() : Base(VariableAttr::status_attr()) {}

        explicit AdderStatus(std::string_view name, std::string_view help, Scope * scope = ScopeInstance::instance()->get_default().get()) : Base(VariableAttr::status_attr()) {
            auto rs = this->expose(name, help, scope);
            if (!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))
                                << "expose Adder failed: " << name << "to scope" << scope->id();
                KLOG(WARNING) << "expose Adder failed: " << name << "to scope" << scope->id();
            }
        }

        ~AdderStatus() { Variable::hide(); }
    };


}  // namespace tally

