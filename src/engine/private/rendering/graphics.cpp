
#include "rendering/graphics.h"

#include <cpputils/logger.hpp>

static std::unique_ptr<GfxInterface> gfx_context_reference;

void Graphics::create(GfxInterface* instance, const WindowParameters& window_parameters)
{
    gfx_context_reference = std::unique_ptr<GfxInterface>(instance);
    gfx_context_reference->init(window_parameters);
}

void Graphics::destroy()
{
    gfx_context_reference->destroy();
    gfx_context_reference = nullptr;
}

GfxInterface* Graphics::get_internal()
{
    if (!gfx_context_reference)
        LOG_FATAL("Error : Graphic context is null");
    return gfx_context_reference.get();
}