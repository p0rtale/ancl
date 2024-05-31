#include <Ancl/Logger/Logger.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>


namespace ancl {

std::shared_ptr<spdlog::logger> Logger::s_Logger;

void Logger::Init() {
    std::vector<spdlog::sink_ptr> logSinks;

    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks[0]->set_pattern("%^[%T] %n: %v%$");

    logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Ancl.log", true));
    logSinks[1]->set_pattern("[%T] [%l] %n: %v");

    s_Logger = std::make_shared<spdlog::logger>("ANCL", std::begin(logSinks), std::end(logSinks));
    
    spdlog::register_logger(s_Logger);
    s_Logger->set_level(spdlog::level::trace);
    s_Logger->flush_on(spdlog::level::trace);
}

}  // namespace ancl
