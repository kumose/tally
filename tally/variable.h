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

#include <string_view>
#include <string>
#include <any>
#include <variant>
#include <turbo/utility/status.h>
#include <turbo/container/flat_hash_map.h>
#include <tally/impl/histogram_bucket.h>
#include <turbo/flags/declare.h>
#include <nlohmann/json.hpp>

namespace tally {

    class StatsReporter;

    class Scope;

    struct VariableType {
        VariableType() = default;

        constexpr explicit VariableType(uint32_t t) : type(t) {}

        constexpr VariableType(const VariableType &t) : type(t.type) {}

        static constexpr uint32_t kCounter = 1;

        static constexpr uint32_t kGauge = 1 << 1;

        static constexpr uint32_t kHistogram = 1 << 2;

        static constexpr uint32_t kFlag = 1 << 3;

        static constexpr uint32_t kMetric = kCounter | kGauge | kHistogram;

        static constexpr uint32_t kCDF = 1 << 26;

        static constexpr uint32_t kSampler = 1 << 27;

        static constexpr uint32_t kWindow = 1 << 28;

        static constexpr uint32_t kCollect = 1 << 29;

        static constexpr uint32_t kStatus = 1 << 30;
        //00000000
        static constexpr uint32_t kEmpty = 0;

        constexpr static VariableType gauge_type() {
            return VariableType{kGauge};
        }

        constexpr static VariableType counter_type() {
            return VariableType{kCounter};
        }

        constexpr static VariableType histogram_type() {
            return VariableType{kHistogram};
        }

        constexpr static VariableType status_type() {
            return VariableType{kStatus};
        }

        constexpr static VariableType flag_type() {
            return VariableType{kFlag};
        }

        constexpr static VariableType empty_type() {
            return VariableType{kEmpty};
        }

        constexpr static VariableType window_type() {
            return VariableType{kWindow};
        }

        constexpr static VariableType collect_type() {
            return VariableType{kCollect};
        }

        constexpr static VariableType sampler_type() {
            return VariableType{kSampler};
        }

        constexpr static VariableType cdf_type() {
            return VariableType{kCDF};
        }

        [[nodiscard]] bool is_empty() const {
            return type & kEmpty;
        }

        [[nodiscard]] bool is_counter() const {
            return type & kCounter;
        }

        [[nodiscard]] bool is_gauge() const {
            return type & kGauge;
        }

        [[nodiscard]] bool is_histogram() const {
            return type & kHistogram;
        }

        [[nodiscard]] bool is_flag() const {
            return type & kFlag;
        }

        [[nodiscard]] bool is_metric() const {
            return type & kMetric;
        }

        [[nodiscard]] bool is_status() const {
            return type & kStatus;
        }

        [[nodiscard]] bool is_window() const {
            return type & kWindow;
        }

        [[nodiscard]] bool is_collect() const {
            return type & kCollect;
        }

        [[nodiscard]] bool is_sampler() const {
            return type & kSampler;
        }

        [[nodiscard]] bool is_cdf() const {
            return type & kCDF;
        }


        [[nodiscard]] bool operator&(uint32_t v) const {
            return type & v;
        }

        [[nodiscard]] operator uint32_t() const {
            return type;
        }

        VariableType &operator&=(uint32_t v) {
            type = type & v;
            return *this;
        }

        constexpr VariableType &operator=(uint32_t v) {
            type = v;
            return *this;
        }

        constexpr VariableType &operator=(VariableType rhs) {
            type = rhs.type;
            return *this;
        }

        [[nodiscard]] bool operator|(uint32_t v) const {
            return type | v;
        }

        VariableType &operator|=(uint32_t v) {
            type = type | v;
            return *this;
        }

        bool operator==(const VariableType &rhs) {
            return type == rhs.type;
        }

        bool operator!=(const VariableType &rhs) {
            return type != rhs.type;
        }

        uint32_t type{0};
    };

    // Bitwise masks of displayable targets
    enum class DisplayFilter : uint32_t {
        DISPLAY_ON_HTML = 1,
        DISPLAY_ON_PLAIN_TEXT = 2,
        DISPLAY_ON_ALL = 3,
    };

    struct VariableAttr {

        constexpr static VariableAttr gauge_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::gauge_type(), f};
        }

        constexpr static VariableAttr counter_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::counter_type(), f};
        }

        constexpr static VariableAttr histogram_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::histogram_type(), f};
        }

        constexpr static VariableAttr status_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::status_type(), f};
        }

        constexpr static VariableAttr flag_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::flag_type(), f};
        }

        constexpr static VariableAttr window_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::window_type(), f};
        }

        constexpr static VariableAttr collect_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::collect_type(), f};
        }

        constexpr static VariableAttr sampler_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::sampler_type(), f};
        }

        constexpr static VariableAttr cdf_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::cdf_type(), f};
        }


        constexpr static VariableAttr empty_attr(DisplayFilter f = DisplayFilter::DISPLAY_ON_ALL) {
            return VariableAttr{VariableType::gauge_type(), f};
        }

        VariableType type{VariableType::empty_type()};
        DisplayFilter display_filter{DisplayFilter::DISPLAY_ON_ALL};
    };

    struct HistogramSample {
        std::vector<HistogramBucket> buckets;
        double sample_sum{0};
        int64_t sample_count{0};

        bool operator==(const HistogramSample &rhs) const {
            return sample_sum == rhs.sample_sum && sample_count == rhs.sample_count && buckets == rhs.buckets;
        }
    };

    struct MetricSample {
        VariableType type;
        std::variant<double, HistogramSample> value;
        turbo::Time timestamp;

        bool operator==(const MetricSample &rhs) const {
            if (type != rhs.type || timestamp != rhs.timestamp) {
                return false;
            }
            try {
                if (type.is_gauge() || type.is_counter()) {
                    return std::get<double>(value) == std::get<double>(rhs.value);
                } else if (type.is_histogram()) {
                    return std::get<HistogramSample>(value) == std::get<HistogramSample>(rhs.value);
                }
                return false;
            } catch (...) {
                return false;
            }
        }
    };

    struct FlagSample {
        std::string help;
        std::string name;
        std::string default_value;
        std::string current_value;
        bool support_update;

        bool operator==(const FlagSample &rhs) {
            return help == rhs.help && name == rhs.name && default_value == rhs.default_value &&
                   support_update == rhs.support_update;
        }
    };

    struct CollectedSample {
        VariableType type;
        std::any value;
        turbo::Time timestamp;

        [[nodiscard]] bool operator==(const CollectedSample &rhs) const {
            if (type != rhs.type || timestamp != rhs.timestamp) {
                return false;
            }
            try {
                if (type.is_gauge() || type.is_counter()) {
                    return std::any_cast<double>(value) == std::any_cast<double>(rhs.value);
                } else if (type.is_histogram()) {
                    return std::any_cast<HistogramSample>(value) == std::any_cast<HistogramSample>(rhs.value);
                } else if (type.is_status()) {
                    return std::any_cast<std::string>(value) == std::any_cast<std::string>(rhs.value);
                } else if (type.is_flag()) {
                    return std::any_cast<FlagSample>(value) == std::any_cast<FlagSample>(rhs.value);
                }
                return true;
            } catch (...) {
                return false;
            }
        }
    };

    class Variable;

    /// return true mean include the var
    struct VariableFilter {
        virtual ~VariableFilter() = default;

        virtual bool is_member(Variable *) const = 0;
    };

    struct SeriesOptions {
        bool fixed_length{true}; // useless now
        bool test_only{false};
    };

    class Variable {
    public:
        Variable(const Variable &) = delete;

        Variable &operator=(Variable &) = delete;

    public:
        explicit Variable(VariableAttr attr) : _attr(attr) {}

        virtual ~Variable();

        turbo::Status expose(std::string_view name, std::string_view help, Scope *scope = nullptr);

        [[nodiscard]] virtual MetricSample get_metric(const turbo::Time &stamp) const {
            return MetricSample{VariableType::empty_type(), static_cast<double>(0.0), turbo::Time::current_time()};
        }

        virtual void describe(std::ostream &, bool quote_string) const {

        }

        // Describe saved series as a json-string into the stream.
        // The output will be ploted by flot.js
        // Returns 0 on success, 1 otherwise(this variable does not save series).
        virtual turbo::Status
        describe_series(std::ostream &, const SeriesOptions &) const {
            return turbo::unavailable_error("");
        }

        virtual turbo::Status describe_series(nlohmann::ordered_json &) const;

        [[nodiscard]] std::string get_description() const;

        virtual void get_value(std::any *value) const;

        [[nodiscard]] const std::string &name() const {
            return _name;
        }

        [[nodiscard]] const std::string &prefix() const;

        /// full name = prefix+"_"+name
        [[nodiscard]] const std::string &full_name() const {
            return _full_name;
        }

        [[nodiscard]] const turbo::flat_hash_map<std::string, std::string> &tags() const;

        [[nodiscard]] const std::string &help() const {
            return _help;
        }

        [[nodiscard]] VariableType type() const {
            return _attr.type;
        }

        [[nodiscard]] VariableAttr attr() const {
            return _attr;
        }

        [[nodiscard]] DisplayFilter display_filter() const {
            return _attr.display_filter;
        }

        [[nodiscard]] bool is_expose() const {
            return _exposed;
        }

        [[nodiscard]] const Scope *scope() const {
            return _scope;
        }

        // Hide this variable so that it's not counted in *_exposed functions.
        // Returns false if this variable is already hidden.
        // CAUTION!! Subclasses must call hide() manually to avoid displaying
        // a variable that is just destructing.
        [[maybe_unused]] bool hide();

        void set_display_filter(DisplayFilter f) {
            _attr.display_filter = f;
        }

        /// static method
        // Put names of all exposed variables into `names'.
        // If you want to print all variables, you have to go through `names'
        // and call `describe_exposed' on each name. This prevents an iteration
        // from taking the lock too long.
        static void list_exposed(std::vector<std::string> *names,
                                 const VariableFilter *filter = nullptr);

        // Get number of exposed variables.
        static size_t count_exposed(const VariableFilter *filter = nullptr);

        /// the name should be full name
        static std::string describe_exposed(std::string_view name, bool quote_string = false);

        /// the name should be full name
        static turbo::Status describe_exposed(std::string_view name, std::ostream &os, bool quote_string = false);

        /// Describe saved series of variable `name' as a json-string into `os'.
        /// The output will be ploted by flot.js
        /// Returns 0 on success, unaviable when the variable does not save series, not found
        /// otherwise (no variable named so).
        static turbo::Status describe_series_exposed(const std::string &name, std::ostream &, const SeriesOptions &);

        static turbo::Status describe_series_exposed(const std::string &name, nlohmann::ordered_json &);

        static void report(turbo::Nonnull<StatsReporter *> reporter, const turbo::Time &stamp);

    protected:
        virtual turbo::Status expose_impl(std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope);

        void set_type(const VariableType &t);

        void exposed_meta(std::ostream &os);

    private:
        void reset();

    private:
        std::string _name;
        std::string _full_name;
        std::string _help;
        VariableAttr _attr{VariableAttr::empty_attr()};
        bool _exposed{false};
        Scope *_scope{nullptr};
    };
}  // namespace tally

namespace std {

    inline ostream &operator<<(ostream &os, const ::tally::Variable &var) {
        var.describe(os, false);
        return os;
    }
}