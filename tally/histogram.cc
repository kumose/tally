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

#include <tally/histogram.h>

namespace tally {
    Histogram::Histogram() : Variable(VariableAttr::histogram_attr()) {

    }
    Histogram::Histogram(const Buckets &buckets) noexcept
            : Variable(VariableAttr::histogram_attr()), buckets_(create_buckets(buckets)) {
        std::vector<Counter<int64_t>> buckets_tmp(buckets_.size());
        buckets_value_.swap(buckets_tmp);
    }

    Histogram::Histogram(const Buckets &buckets, std::string_view name, std::string_view help, turbo::Nonnull<Scope *> scope) noexcept
    : Variable(VariableAttr::histogram_attr()), buckets_(create_buckets(buckets)){
        std::vector<Counter<int64_t>> buckets_tmp(buckets_.size());
        buckets_value_.swap(buckets_tmp);
        auto rs = expose(name,help,scope);
        if(!rs.ok()) {
            KLOG_IF(FATAL, turbo::get_flag(FLAGS_tally_crash_on_expose_fail))<<"expose Histogram failed: "<<name<<"to scope"<<scope->id();
            KLOG(WARNING)<<"expose Histogram failed: "<<name<<"to scope"<<scope->id();
        }
    }

    Histogram::~Histogram() {
        hide();
    }

    void Histogram::set_buckets(const Buckets &buckets) noexcept {
        if(!buckets_.empty()) {
            return;
        }
        buckets_ = create_buckets(buckets);
        std::vector<Counter<int64_t>> buckets_tmp(buckets_.size());
        buckets_value_.swap(buckets_tmp);
    }

    std::vector<HistogramBucket> Histogram::create_buckets(
            const Buckets &buckets) {
        std::vector<HistogramBucket> histogram_buckets;
        auto const size = buckets.size();
        auto const kind = buckets.kind();
        if (size == 0) {
            histogram_buckets.emplace_back(
                    kind, 0, 1, std::numeric_limits<double>::min(),
                                    std::numeric_limits<double>::max());
        } else {
            histogram_buckets.reserve(size);

            auto lower_bound = std::numeric_limits<double>::min();
            for (auto it = buckets.begin(); it != buckets.end(); it++) {
                auto upper_bound = *it;
                auto index = static_cast<uint64_t>(std::distance(buckets.begin(), it));
                histogram_buckets.emplace_back(kind, index, buckets.size(),
                                                            lower_bound, upper_bound);
                lower_bound = upper_bound;
            }

            // Add a catch-all bucket for anything past the last bucket.
            histogram_buckets.emplace_back(
                    kind, buckets.size(), buckets.size(), lower_bound,
                                    std::numeric_limits<double>::max());
        }

        return histogram_buckets;
    }

    void Histogram::record(double val) noexcept {
        if(buckets_.empty()) {
            return;
        }
        // Find the first bucket who's upper bound is greater than val.
        auto it = std::upper_bound(buckets_.begin(), buckets_.end(), val,
                                   [](double lhs, const HistogramBucket &rhs) {
                                       return lhs < rhs.upper_bound;
                                   });
       buckets_value_[it->bucket_id].increment(1);
       sample_count_.increment(1);
       sample_sum_.increment(val);
    }

    std::vector<HistogramBucket> Histogram::get_value() const {
        std::vector<HistogramBucket> result = buckets_;
        for(size_t i = 0; i < result.size(); i++) {
            result[i].value = buckets_value_[i].get_value();
        }
        return result;
    }

}  // namespace tally
