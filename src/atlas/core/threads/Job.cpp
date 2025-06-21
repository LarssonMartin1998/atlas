#include "core/threads/Job.hpp"

namespace atlas::core {
auto Job::is_running() const -> bool {
    return task.has_value();
}
} // namespace atlas::core
