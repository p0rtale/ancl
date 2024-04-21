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
