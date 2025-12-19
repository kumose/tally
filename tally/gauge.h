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
#include <tally/status.h>
#include <tally/passive_status.h>
#include <tally/impl/reducer.h>
#include <tally/flag.h>
#include <tally/scope.h>

namespace tally {

    template <typename T, typename Enabler = void>
    class Gauge;

    template <typename T>
    class Gauge<T, typename std::enable_if<detail::is_atomical<T>::value>::type>
            : public Variable {
    public:
        struct PlaceHolderOp {
            void operator()(T&, const T&) const {}
        };
        class SeriesSampler : public detail::Sampler {
        public:
            typedef typename std::conditional<
                    true, detail::AddTo<T>, PlaceHolderOp>::type Op;
            explicit SeriesSampler(Gauge* owner)
                    : _owner(owner), _series(Op()) {}
            void take_sample() { _series.append(_owner->get_value()); }
            void describe(std::ostream& os) { _series.describe(os, nullptr); }
        private:
            Gauge* _owner;
            detail::Series<T, Op> _series;
        };

    public:
        Gauge() : Variable(VariableAttr::gauge_attr()),_series_sampler(nullptr) {}
        Gauge(const T& value) : Variable(VariableAttr::gauge_attr()),_value(value), _series_sampler(nullptr) { }
        Gauge(std::string_view name, std::string_view help, const T& value, Scope* scope = ScopeInstance::instance()->get_default().get())
                : Variable(VariableAttr::gauge_attr()), _value(value), _series_sampler(nullptr) {
            auto rs = this->expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose Status failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose Status failed: "<<name<<"to scope"<<scope->id();
            }
        }
        ~Gauge() {
            hide();
            if (_series_sampler) {
                _series_sampler->destroy();
                _series_sampler = nullptr;
            }
        }

        void describe(std::ostream& os, bool /*quote_string*/) const override {
            os << get_value();
        }

        void get_value(std::any* value) const override {
            *value = get_value();
        }

        T get_value() const {
            return _value.load(std::memory_order_relaxed);
        }

        void set_value(const T& value) {
            _value.store(value, std::memory_order_relaxed);
        }

        void update(T v) {
            set_value(v);
        }
        MetricSample get_metric(const turbo::Time &stamp) const override {
            return MetricSample{VariableType::gauge_type(), static_cast<double>(this->get_value()), stamp};
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
        turbo::Status expose_impl(std::string_view name, std::string_view help, turbo::Nonnull<Scope*> scope) override {
            auto rc = Variable::expose_impl(name, help, scope);
            if (rc.ok() &&
                _series_sampler == nullptr &&
                turbo::get_flag(FLAGS_tally_save_series)) {
                _series_sampler = new SeriesSampler(this);
                _series_sampler->schedule();
            }
            return rc;
        }

    private:
        std::atomic<T> _value;
        SeriesSampler* _series_sampler;
    };

    using SimpleGauge = Gauge<double>;

    template <typename T, typename Enabler = void>
    class MaxerGauge;
    template<typename T>
    class MaxerGauge<T, typename std::enable_if<detail::is_atomical<T>::value>::type> : public Reducer<T, detail::MaxTo<T> > {
    public:
        typedef Reducer<T, detail::MaxTo<T> > Base;
        typedef T value_type;
        typedef typename Base::sampler_type sampler_type;
    public:
        MaxerGauge() : Base(VariableAttr::gauge_attr() , std::numeric_limits<T>::min()) {}

        explicit MaxerGauge(T v) : Base(VariableAttr::gauge_attr() , v) {}

        explicit MaxerGauge(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope = ScopeInstance::instance()->get_default().get())
                : Base(VariableAttr::gauge_attr(),std::numeric_limits<T>::min()) {
            auto rs = this->expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose MaxerGauge failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose MaxerGauge failed: "<<name<<"to scope"<<scope->id();
            }
        }
        void update(T v) {
            *this<<v;
        }

        ~MaxerGauge() { Variable::hide(); }

        MetricSample get_metric(const turbo::Time &stamp) const override {
            return MetricSample{VariableType::gauge_type(), static_cast<double>(this->get_value()), stamp};
        }
    };


    template <typename T, typename Enabler = void>
    class MinerGauge;
    template<typename T>
    class MinerGauge<T, typename std::enable_if<detail::is_atomical<T>::value>::type> : public Reducer<T, detail::MinTo<T> > {
    public:
        typedef Reducer<T, detail::MinTo<T> > Base;
        typedef T value_type;
        typedef typename Base::sampler_type sampler_type;
    public:
        MinerGauge() : Base(VariableAttr::gauge_attr(), std::numeric_limits<T>::max()) {}

        explicit MinerGauge(T v) : Base(VariableAttr::gauge_attr(), v) {}

        explicit MinerGauge(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope = ScopeInstance::instance()->get_default().get())
                : Base(VariableAttr::gauge_attr(),std::numeric_limits<T>::max()) {
            auto rs = this->expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose MinerGauge failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose MinerGauge failed: "<<name<<"to scope"<<scope->id();
            }
        }

        void update(T v) {
            *this<<v;
        }

        MetricSample get_metric(const turbo::Time &stamp) const override {
            return MetricSample{VariableType::gauge_type(), static_cast<double>(this->get_value()), stamp};
        }

        ~MinerGauge() { Variable::hide(); }
    };


    // Display a updated-by-need value. This is done by passing in an user callback
    // which is called to produce the value.
    // Example:
    //   int print_number(void* arg) {
    //      ...
    //      return 5;
    //   }
    //
    //   // number1 : 5
    //   int var = 5;
    //   tally::FuncGauge<int> status1("number1", [var]()print_number(void* arg) {
    //         ...
    //       return var;
    //     });

    template<typename T, typename Enable = void>
    class FuncGauge;
    template<typename T>
    class FuncGauge<T, typename std::enable_if<std::is_integral_v<T> || std::is_floating_point_v<T>>::type> : public Variable {
    public:
        typedef T value_type;
        typedef detail::ReducerSampler<FuncGauge, T, detail::AddTo<T>,
                detail::MinusFrom<T> > sampler_type;

        class SeriesSampler : public detail::Sampler {
        public:
            typedef detail::AddTo<T> Op;

            explicit SeriesSampler(FuncGauge *owner)
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
            FuncGauge *_owner;
            std::string *_vector_names;
            detail::Series<T, Op> _series;
        };

    public:
        FuncGauge(std::string_view name,
                      std::string_view help,
                      std::function<T(void)> &&getfn, turbo::Nonnull<Scope*> scope = ScopeInstance::instance()->get_default().get())
                : Variable(VariableAttr::gauge_attr()), _get_func(getfn), _sampler(nullptr), _series_sampler(nullptr) {
            auto rs = expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose FuncGauge failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose FuncGauge failed: "<<name<<"to scope"<<scope->id();
            }
        }

        FuncGauge(std::function<T()> &&func)
                : Variable(VariableAttr::gauge_attr()),_get_func(func), _sampler(nullptr), _series_sampler(nullptr) {
        }

        ~FuncGauge() {
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

        void get_value(std::any* value) const override {
            if (_get_func) {
                *value = _get_func();
            } else {
                *value = T();
            }
        }

        T get_value() const {
            return (_get_func ? _get_func() : T());
        }

        sampler_type *get_sampler() {
            if (nullptr == _sampler) {
                _sampler = new sampler_type(this);
                _sampler->schedule();
            }
            return _sampler;
        }

        detail::AddTo<T> op() const { return detail::AddTo<T>(); }

        detail::MinusFrom<T> inv_op() const { return detail::MinusFrom<T>(); }

        T reset() {
            KCHECK(false) << "FuncGauge::reset() should never be called, abort";
            abort();
        }

        MetricSample get_metric(const turbo::Time &stamp) const override {
            return MetricSample{VariableType::gauge_type(), static_cast<double>(get_value()), stamp};
        }

        turbo::Status describe_series(std::ostream& os, const SeriesOptions& options) const override {
            if (_series_sampler == NULL) {
                return turbo::unavailable_error("");
            }
            if (!options.test_only) {
                _series_sampler->describe(os);
            }
            return turbo::OkStatus();
        }
    protected:
        turbo::Status expose_impl(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope) override {
            auto rc = Variable::expose_impl(name, help, scope);
            if (rc.ok()&&
                _series_sampler == nullptr &&
                turbo::get_flag(FLAGS_tally_save_series)) {
                _series_sampler = new SeriesSampler(this);
                _series_sampler->schedule();
            }
            return rc;
        }

    private:
        std::function<T(void)> _get_func;
        sampler_type *_sampler;
        SeriesSampler *_series_sampler;
    };

    class FlagGauge : public FlagStatus {
    public:
        template<typename T, typename std::enable_if<std::is_floating_point_v<T>||std::is_integral_v<T>, int>::type = 0>
        FlagGauge(turbo::Flag<T>* flag, turbo::Nonnull<Scope *> scope = ScopeInstance::instance()->get_default().get()) : FlagStatus(flag->name(), flag->help(),scope) {

        }

        MetricSample get_metric(const turbo::Time &stamp) const override {
            return MetricSample{VariableType::gauge_type(), static_cast<double>(0.0), stamp};
        }
    private:

    };

    struct Stat {
        Stat() : sum(0), num(0) {}

        Stat(int64_t sum2, int64_t num2) : sum(sum2), num(num2) {}

        int64_t sum;
        int64_t num;

        int64_t get_average_int() const {
            //num can be changed by sampling thread, use tmp_num
            int64_t tmp_num = num;
            if (tmp_num == 0) {
                return 0;
            }
            return sum / (int64_t) tmp_num;
        }

        double get_average_double() const {
            int64_t tmp_num = num;
            if (tmp_num == 0) {
                return 0.0;
            }
            return (double) sum / (double) tmp_num;
        }

        Stat operator-(const Stat &rhs) const {
            return Stat(sum - rhs.sum, num - rhs.num);
        }

        void operator-=(const Stat &rhs) {
            sum -= rhs.sum;
            num -= rhs.num;
        }

        Stat operator+(const Stat &rhs) const {
            return Stat(sum + rhs.sum, num + rhs.num);
        }

        void operator+=(const Stat &rhs) {
            sum += rhs.sum;
            num += rhs.num;
        }
    };

    inline std::ostream &operator<<(std::ostream &os, const Stat &s) {
        const int64_t v = s.get_average_int();
        if (v != 0) {
            return os << v;
        } else {
            return os << s.get_average_double();
        }
    }

    // For calculating average of numbers.
    // Example:
    //   IntRecorder latency;
    //   latency << 1 << 3 << 5;
    //   KCHECK_EQ(3, latency.average());
    class AverageGauge : public Variable {
    public:
        // Compressing format:
        // | 20 bits (unsigned) | sign bit | 43 bits |
        //       num                   sum
        const static size_t SUM_BIT_WIDTH = 44;
        const static uint64_t MAX_SUM_PER_THREAD = (1ul << SUM_BIT_WIDTH) - 1;
        const static uint64_t MAX_NUM_PER_THREAD = (1ul << (64ul - SUM_BIT_WIDTH)) - 1;
        static_assert(SUM_BIT_WIDTH > 32 && SUM_BIT_WIDTH < 64,
                      "SUM_BIT_WIDTH_must_be_between_33_and_63");

        struct AddStat {
            void operator()(Stat &s1, const Stat &s2) const { s1 += s2; }
        };

        struct MinusStat {
            void operator()(Stat &s1, const Stat &s2) const { s1 -= s2; }
        };

        typedef Stat value_type;
        typedef detail::ReducerSampler<AverageGauge, Stat,
                AddStat, MinusStat> sampler_type;

        typedef Stat SampleSet;

        struct AddToStat {
            void operator()(Stat &lhs, uint64_t rhs) const {
                lhs.sum += _extend_sign_bit(_get_sum(rhs));
                lhs.num += _get_num(rhs);
            }
        };

        typedef detail::AgentCombiner<Stat, uint64_t, AddToStat> combiner_type;
        typedef combiner_type::Agent agent_type;

        AverageGauge() : Variable(VariableAttr::gauge_attr()), _sampler(nullptr) {

        }
        AverageGauge(std::string_view name, std::string_view help, Scope *scope) : Variable(VariableAttr::gauge_attr()), _sampler(nullptr) {
            auto rs = expose(name, help, scope);
            if(!rs.ok()) {
                KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose AverageGauge failed: "<<name<<"to scope"<<scope->id();
                KLOG(WARNING)<<"expose AverageGauge failed: "<<name<<"to scope"<<scope->id();
            }
        }

        ~AverageGauge() {
            hide();
            if (_sampler) {
                _sampler->destroy();
                _sampler = nullptr;
            }
        }

        // Note: The input type is acutally int. Use int64_t to check overflow.
        AverageGauge &operator<<(int64_t/*note*/ sample);

        int64_t average() const {
            return _combiner.combine_agents().get_average_int();
        }

        double average(double) const {
            return _combiner.combine_agents().get_average_double();
        }

        Stat get_value() const {
            return _combiner.combine_agents();
        }

        Stat reset() {
            return _combiner.reset_all_agents();
        }

        AddStat op() const { return AddStat(); }

        MinusStat inv_op() const { return MinusStat(); }

        void describe(std::ostream &os, bool /*quote_string*/) const override {
            os << get_value();
        }

        void update(int64_t v) {
            *this<<v;
        }

        bool valid() const { return _combiner.valid(); }

        sampler_type *get_sampler() {
            if (nullptr == _sampler) {
                _sampler = new sampler_type(this);
                _sampler->schedule();
            }
            return _sampler;
        }

        // This name is useful for printing overflow log in operator<< since
        // IntRecorder is often used as the source of data and not exposed.
        void set_debug_name(const std::string_view &name) {
            _debug_name.assign(name.data(), name.size());
        }

        MetricSample get_metric(const turbo::Time &stamp) const override {
            return MetricSample{VariableType::gauge_type(), static_cast<double>(get_value().get_average_double()), stamp};
        }

    private:
        // TODO: The following numeric functions should be independent utils
        static uint64_t _get_sum(const uint64_t n) {
            return (n & MAX_SUM_PER_THREAD);
        }

        static uint64_t _get_num(const uint64_t n) {
            return n >> SUM_BIT_WIDTH;
        }

        // Fill all the first (64 - SUM_BIT_WIDTH + 1) bits with 1 if the sign bit is 1
        // to represent a complete 64-bit negative number
        // Check out http://en.wikipedia.org/wiki/Signed_number_representations if
        // you are confused
        static int64_t _extend_sign_bit(const uint64_t sum) {
            return (((1ul << (64ul - SUM_BIT_WIDTH + 1)) - 1)
                    * ((1ul << (SUM_BIT_WIDTH - 1) & sum)))
                   | (int64_t) sum;
        }

        // Convert complement into a |SUM_BIT_WIDTH|-bit unsigned integer
        static uint64_t _get_complement(int64_t n) {
            return n & (MAX_SUM_PER_THREAD);
        }

        static uint64_t _compress(const uint64_t num, const uint64_t sum) {
            return (num << SUM_BIT_WIDTH)
                   // There is a redundant '1' in the front of sum which was
                   // combined with two negative number, so truncation has to be
                   // done here
                   | (sum & MAX_SUM_PER_THREAD);
        }

        // Check whether the sum of the two integer overflows the range of signed
        // integer with the width of SUM_BIT_WIDTH, which is
        // [-2^(SUM_BIT_WIDTH -1), 2^(SUM_BIT_WIDTH -1) - 1) (eg. [-128, 127) for
        // signed 8-bit integer)
        static bool _will_overflow(const int64_t lhs, const int rhs) {
            return
                // Both integers are positive and the sum is larger than the largest
                // number
                    ((lhs > 0) && (rhs > 0)
                     && (lhs + rhs > ((int64_t) MAX_SUM_PER_THREAD >> 1)))
                    // Or both integers are negative and the sum is less than the lowest
                    // number
                    || ((lhs < 0) && (rhs < 0)
                        && (lhs + rhs < (-((int64_t) MAX_SUM_PER_THREAD >> 1)) - 1))
                // otherwise the sum cannot overflow iff lhs does not overflow
                // because |sum| < |lhs|
                    ;
        }

    private:
        combiner_type _combiner;
        sampler_type *_sampler;
        std::string _debug_name;
    };

    inline AverageGauge &AverageGauge::operator<<(int64_t sample) {
        if (TURBO_UNLIKELY((int64_t) (int) sample != sample)) {
            const char *reason = nullptr;
            if (sample > std::numeric_limits<int>::max()) {
                reason = "overflows";
                sample = std::numeric_limits<int>::max();
            } else {
                reason = "underflows";
                sample = std::numeric_limits<int>::min();
            }
            // Truncate to be max or min of int. We're using 44 bits to store the
            // sum thus following aggregations are not likely to be over/underflow.
            if (!name().empty()) {
                KLOG(WARNING) << "Input=" << sample << " to `" << name()
                             << "\' " << reason;
            } else if (!_debug_name.empty()) {
                KLOG(WARNING) << "Input=" << sample << " to `" << _debug_name
                             << "\' " << reason;
            } else {
                KLOG(WARNING) << "Input=" << sample << " to IntRecorder("
                             << (void *) this << ") " << reason;
            }
        }
        agent_type *agent = _combiner.get_or_create_tls_agent();
        if (TURBO_UNLIKELY(!agent)) {
            KLOG(FATAL) << "Fail to create agent";
            return *this;
        }
        uint64_t n;
        agent->element.load(&n);
        const uint64_t complement = _get_complement(sample);
        uint64_t num;
        uint64_t sum;
        do {
            num = _get_num(n);
            sum = _get_sum(n);
            if (TURBO_UNLIKELY((num + 1 > MAX_NUM_PER_THREAD) ||
                               _will_overflow(_extend_sign_bit(sum), sample))) {
                // Although agent->element might have been cleared at this
                // point, it is just OK because the very value is 0 in
                // this case
                agent->combiner->commit_and_clear(agent);
                sum = 0;
                num = 0;
                n = 0;
            }
        } while (!agent->element.compare_exchange_weak(
                n, _compress(num + 1, sum + complement)));
        return *this;
    }
}  // namespace tally
