#include <Ancl/Preprocessor/Streams/Streams.hpp>

#include <Ancl/Base.hpp>


namespace preproc {

StreamStack::StreamStack(OnPushCallbackT pushCallback, OnPopCallbackT popCallback)
    : m_PushCallback(std::move(pushCallback)), m_PopCallback(std::move(popCallback)) {}

void StreamStack::PushStream(const std::string& filename, size_t line) {
    if (!m_Streams.empty()) {
        auto& topStream = m_Streams.top();
        topStream.second.second = line;
    }

    auto stream = std::make_unique<std::ifstream>(filename);
    m_PushCallback(stream);
    m_Streams.push({std::move(stream), std::make_pair(filename, 1)});
}

std::pair<std::string, size_t> StreamStack::PopStream() {
    auto streamsSize = m_Streams.size();
    if (streamsSize == 0) {
        return {std::string{}, 0};
    }
    m_Streams.pop();
    if (streamsSize == 1) {
        return {std::string{}, 0};
    }

    auto& stream = m_Streams.top().first;
    m_PopCallback(stream);

    auto [filename, line] = m_Streams.top().second;

    return {filename, line}; 
}

}  // namespace preproc
