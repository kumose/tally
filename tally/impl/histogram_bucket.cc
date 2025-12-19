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

#include <tally/impl/histogram_bucket.h>

namespace tally {

    HistogramBucket::HistogramBucket(Buckets::Kind k, uint64_t id,
                                     uint64_t buckets, double lower,
                                     double upper, int64_t v)
            : kind(k),
              bucket_id(id),
              num_buckets(buckets),
              lower_bound(lower),
              upper_bound(upper),
              value(v) {}


    bool HistogramBucket::operator==(const HistogramBucket &rhs) const {
        return kind == rhs.kind &&
        bucket_id == rhs.bucket_id &&
        num_buckets == rhs.num_buckets &&
        lower_bound == rhs.lower_bound &&
        upper_bound == rhs.upper_bound &&
        value == rhs.value;
    }
}  // namespace tally
/*
bool operator==(const std::vector<tally::HistogramBucket>&lhs, const std::vector<tally::HistogramBucket>&rhs) {
    if(lhs.size() != rhs.size()) {
        return false;
    }
    for(size_t i =0 ; i < lhs.size(); i++) {
        if(lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}
*/