#include <fivemlinux/fivemlinux.h>

namespace fivemlinux {

FiveMLinux::FiveMLinux() = default;
FiveMLinux::~FiveMLinux() = default;

bool FiveMLinux::initialize() {
    return true;
}

void FiveMLinux::shutdown() {
}

} // namespace fivemlinux
