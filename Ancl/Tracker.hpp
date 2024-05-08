#pragma once


template <typename T>
class Tracker {
public:
    Tracker() = default;
    
    Tracker(const Tracker&) = delete;
    Tracker(Tracker&&) = delete;

    Tracker& operator=(const Tracker&) = delete;
    Tracker& operator=(Tracker&&) = delete;

    ~Tracker() {
        DeallocateAll();
    }

    template<typename U, typename... Args>
    U* Allocate(Args&&... args) {
        static_assert(std::is_base_of_v<T, U>);
        U* result = new U(args...);
        m_Allocated.push_back(result);
        return result;
    }

    void DeallocateAll() {
        for (T* entry : m_Allocated) {
            delete entry;
        }
        m_Allocated.clear();
    }

private:
    std::vector<T*> m_Allocated;
};
