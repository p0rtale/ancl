#pragma once

#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <Ancl/Logger/Logger.hpp>


template<typename T>
using TScopePtr = std::unique_ptr<T>;

template<typename T, typename... Args>
constexpr TScopePtr<T> CreateScope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

namespace std {

    namespace {

        // Code from boost
        // Reciprocal of the golden ratio helps spread entropy
        //     and handles duplicates.
        template <typename T>
        inline void hash_combine(std::size_t& seed, const T& v) {
            seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        template <typename Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct HashValueImpl {
            static void apply(size_t& seed, const Tuple& tuple) {
                HashValueImpl<Tuple, Index-1>::apply(seed, tuple);
                hash_combine(seed, get<Index>(tuple));
            }
        };

        template <typename Tuple>
        struct HashValueImpl<Tuple, 0> {
            static void apply(size_t& seed, const Tuple& tuple) {
                hash_combine(seed, get<0>(tuple));
            }
        };

    }

    template <typename... TT>
    struct hash<std::tuple<TT...>> {
        size_t operator()(const std::tuple<TT...>& tt) const {                                              
            size_t seed = 0;                             
            HashValueImpl<std::tuple<TT...>>::apply(seed, tt);
            return seed;
        }

    };

}  // namespace std
