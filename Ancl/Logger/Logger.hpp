#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>


namespace ancl {

class Logger {
public:
    static void init();

    static std::shared_ptr<spdlog::logger> getLogger() {
        return s_Logger;
    }

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

}  // namespace ancl


#define ANCL_TRACE(...)    ::ancl::Logger::getLogger()->trace(__VA_ARGS__)
#define ANCL_INFO(...)     ::ancl::Logger::getLogger()->info(__VA_ARGS__)
#define ANCL_WARN(...)     ::ancl::Logger::getLogger()->warn(__VA_ARGS__)
#define ANCL_ERROR(...)    ::ancl::Logger::getLogger()->error(__VA_ARGS__)
#define ANCL_CRITICAL(...) ::ancl::Logger::getLogger()->critical(__VA_ARGS__)
