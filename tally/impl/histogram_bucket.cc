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