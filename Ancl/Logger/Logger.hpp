#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>


namespace ancl {

class Logger {
public:
    static void Init();

    static std::shared_ptr<spdlog::logger> GetLogger() {
        return s_Logger;
    }

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

}  // namespace ancl


#define ANCL_TRACE(...)    ::ancl::Logger::GetLogger()->trace(__VA_ARGS__)
#define ANCL_INFO(...)     ::ancl::Logger::GetLogger()->info(__VA_ARGS__)
#define ANCL_WARN(...)     ::ancl::Logger::GetLogger()->warn(__VA_ARGS__)
#define ANCL_ERROR(...)    ::ancl::Logger::GetLogger()->error(__VA_ARGS__)
#define ANCL_CRITICAL(...) ::ancl::Logger::GetLogger()->critical(__VA_ARGS__)
