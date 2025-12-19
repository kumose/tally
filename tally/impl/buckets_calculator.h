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

namespace tally {

    class BucketsCalculator {
    public:
        enum class Growth {
            Linear,
            Exponential,
        };

        BucketsCalculator(Growth growth, double start, double update);

        double calculate(uint64_t index) const;

        bool operator==(BucketsCalculator other) const;

        bool operator!=(BucketsCalculator other) const {
            return !(*this == other);
        }

    private:
        const Growth growth_;
        const double start_;
        const double update_;
    };

}  // namespace tally
