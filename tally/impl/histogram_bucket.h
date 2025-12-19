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
