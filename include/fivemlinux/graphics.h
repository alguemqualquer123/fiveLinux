#pragma once

#include <string>

namespace fivemlinux {
namespace graphics {

class Graphics {
public:
    Graphics();
    ~Graphics();

    bool initialize();
    void shutdown();
};

} // namespace graphics
} // namespace fivemlinux
