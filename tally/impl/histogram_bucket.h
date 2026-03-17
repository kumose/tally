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

#include <string>
#include <turbo/container/flat_hash_map.h>
#include <vector>

#include <tally/buckets.h>

namespace tally {

    struct HistogramBucket {
        HistogramBucket(Buckets::Kind kind, uint64_t bucket_id, uint64_t num_buckets,
                        double lower_bound, double upper_bound, int64_t value = 0);

        Buckets::Kind kind;
        uint64_t bucket_id;
        uint64_t num_buckets;
        double lower_bound;
        double upper_bound;
        int64_t value;
        bool operator==(const HistogramBucket &rhs) const;
        bool operator!=(const HistogramBucket &rhs) const {
            return !(*this == rhs);
        }
    };

}  // namespace tally

//bool operator==(const std::vector<tally::HistogramBucket>&, const std::vector<tally::HistogramBucket>&);
