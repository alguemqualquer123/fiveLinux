#include "fivemlinux/graphics.h"

namespace fivemlinux {
namespace graphics {

Graphics::Graphics() = default;
Graphics::~Graphics() = default;

bool Graphics::initialize() {
    return true;
}

void Graphics::shutdown() {
}

} // namespace graphics
} // namespace fivemlinux
