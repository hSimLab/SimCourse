#ifndef __LIB_INCLUDE_TIMER_HH__
#define __LIB_INCLUDE_TIMER_HH__

#include <chrono>

namespace Time {
using std::chrono::microseconds;

class Timer final {
    using clock_t = std::chrono::high_resolution_clock;

    std::chrono::time_point<clock_t> beg;

public:
    Timer() : beg(clock_t::now()) {}

    void reset_time() { beg = clock_t::now(); }

    template <typename T>
    double elapsed() const {
        return std::chrono::duration_cast<T>(clock_t::now() - beg).count();
    }
};
}  // namespace Time

#endif  // __LIB_INCLUDE_TIMER_HH__
