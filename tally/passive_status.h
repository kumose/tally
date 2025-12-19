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

#include <tally/variable.h>
#include <tally/impl/reducer.h>
#include <tally/config.h>

namespace tally {

    // Display a updated-by-need value. This is done by passing in an user callback
    // which is called to produce the value.
    // Example:
    //   // number : 5
    //   tally::PassiveStatus<int> status2([](){
    //      return 5;
    //   });

    template<typename Tp, typename Enable = void>
    class PassiveStatus;

    template<typename Tp>
    class PassiveStatus<Tp, typename std::enable_if<!std::is_integral_v<Tp> &&
                                                    !std::is_floating_point_v<Tp> &&
                                                    !std::is_same_v<Tp, std::string>
    >::type> : public Variable {
    public:
        typedef Tp value_type;
        typedef detail::ReducerSampler<PassiveStatus, Tp, detail::AddTo<Tp>,
                detail::MinusFrom<Tp> > sampler_type;

        struct PlaceHolderOp {
            void operator()(Tp &, const Tp &) const {}
        };

        static const bool ADDITIVE = is_vector<Tp>::value;

        class SeriesSampler : public detail::Sampler {
        public:
            typedef typename std::conditional<
                    ADDITIVE, detail::AddTo<Tp>, PlaceHolderOp>::type Op;

            explicit SeriesSampler(PassiveStatus *owner)
                    : _owner(owner), _vector_names(nullptr), _series(Op()) {}

            ~SeriesSampler() {
                delete _vector_names;
            }

            void take_sample() override { _series.append(_owner->get_value()); }

            void describe(std::ostream &os) { _series.describe(os, _vector_names); }

            void set_vector_names(const std::string &names) {
                if (_vector_names == nullptr) {
                    _vector_names = new std::string;
                }
                *_vector_names = names;
            }

        private:
            PassiveStatus *_owner;
            std::string *_vector_names;
            detail::Series<Tp, Op> _series;
        };

    public:
        PassiveStatus(std::string_view &name,
                      const std::string_view &help,
                      std::function<Tp(void)> &&getfn,  Scope *scope = ScopeInstance::instance()->get_default().get())
                : Variable(VariableAttr::status_attr()), _get_func(getfn),
                  _sampler(nullptr), _series_sampler(nullptr) {
            auto rs = expose(name, help, scope);
            if (!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))
                                << "expose PassiveStatus failed: " << name << "to scope" << scope->id();
                KLOG(WARNING) << "expose PassiveStatus failed: " << name << "to scope" << scope->id();
            }
        }

        PassiveStatus(std::function<Tp()> &&func)
                : Variable(VariableAttr::status_attr()), _get_func(func),
                  _sampler(nullptr), _series_sampler(nullptr) {
        }

        ~PassiveStatus() {
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

        int set_vector_names(const std::string &names) {
            if (_series_sampler) {
                _series_sampler->set_vector_names(names);
                return 0;
            }
            return -1;
        }

        void describe(std::ostream &os, bool /*quote_string*/) const override {
            os << get_value();
        }

        void get_value(std::any *value) const override {
            if (_get_func) {
                *value = _get_func();
            } else {
                *value = Tp();
            }
        }

        Tp get_value() const {
            return (_get_func ? _get_func() : Tp());
        }

        sampler_type *get_sampler() {
            if (nullptr == _sampler) {
                _sampler = new sampler_type(this);
                _sampler->schedule();
            }
            return _sampler;
        }

        detail::AddTo<Tp> op() const { return detail::AddTo<Tp>(); }

        detail::MinusFrom<Tp> inv_op() const { return detail::MinusFrom<Tp>(); }

        Tp reset() {
            KCHECK(false) << "PassiveStatus::reset() should never be called, abort";
            abort();
        }

    protected:
        turbo::Status
        expose_impl(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope) override {
            auto rc = Variable::expose_impl(name, help, scope);
            if (ADDITIVE &&
                rc.ok() &&
                _series_sampler == nullptr &&
                turbo::get_flag(FLAGS_tally_save_series)) {
                _series_sampler = new SeriesSampler(this);
                _series_sampler->schedule();
            }
            return rc;
        }

    private:
        std::function<Tp(void)> _get_func;
        sampler_type *_sampler;
        SeriesSampler *_series_sampler;
    };

    // ccover g++ may complain about ADDITIVE is undefined unless it's
    // explicitly declared here.
    template<typename Tp> const bool PassiveStatus<Tp, typename std::enable_if<!std::is_integral_v<Tp> &&
                                                                               !std::is_floating_point_v<Tp> &&
                                                                               !std::is_same_v<Tp, std::string>
    >::type>::ADDITIVE;

    // Specialize std::string for using std::ostream& as a more friendly
    // interface for user's callback.
    template<typename T>
    class PassiveStatus<T, typename std::enable_if<std::is_same_v<T, std::string>>::type> : public Variable {
    public:
        // NOTE: You must be very careful about lifetime of `arg' which should be
        // valid during lifetime of PassiveStatus.
        PassiveStatus(std::string_view name, std::string_view help,
                      std::function<void(std::ostream &)> &&func,
                      Scope *scope = ScopeInstance::instance()->get_default().get())
                : Variable(VariableAttr::status_attr()), _print(func) {
            auto rs = expose(name, help, scope);
            if (!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))
                                << "expose PassiveStatus failed: " << name << "to scope" << scope->id();
                KLOG(WARNING) << "expose PassiveStatus failed: " << name << "to scope" << scope->id();
            }
        }

        PassiveStatus(std::function<void(std::ostream &)> &&func)
                : Variable(VariableAttr::status_attr()), _print(func) {}

        ~PassiveStatus() {
            hide();
        }

        void describe(std::ostream &os, bool quote_string) const override {
            if (quote_string) {
                if (_print) {
                    os << '"';
                    _print(os);
                    os << '"';
                } else {
                    os << "\"null\"";
                }
            } else {
                if (_print) {
                    _print(os);
                } else {
                    os << "null";
                }
            }
        }

    private:
        std::function<void(std::ostream &)> _print;
    };

    template<typename Tp>
    class BasicPassiveStatus : public PassiveStatus<Tp> {
    public:
        BasicPassiveStatus(std::string_view name, std::string_view help,
                           std::function<Tp(void)> &&func, Scope * scope = ScopeInstance::instance()->get_default().get())
                : PassiveStatus<Tp>(name, help, scope, std::forward<std::function<Tp(void)>>(func)) {}

        BasicPassiveStatus(std::function<Tp(void)> &&func)
                : PassiveStatus<Tp>(func) {}
    };

    template<>
    class BasicPassiveStatus<std::string> : public PassiveStatus<std::string> {
    public:
        BasicPassiveStatus(std::string_view name, std::string_view help,
                           std::function<void(std::ostream &)> &&func, Scope * scope = ScopeInstance::instance()->get_default().get())
                : PassiveStatus<std::string>(name, help,
                                             std::forward<std::function<void(std::ostream &)>>(func), scope) {}

        BasicPassiveStatus(std::function<void(std::ostream &)> &&func)
                : PassiveStatus<std::string>(std::forward<std::function<void(std::ostream &)>>(func)) {}
    };


}  // namespace tally
