#pragma once

namespace kvstore {
    static constexpr size_t NUM_SHARDS = 8;
    static constexpr size_t TOTAL_CAPACITY = 1024;
    static constexpr size_t LOCAL_CAPACITY = TOTAL_CAPACITY / NUM_SHARDS;
}
