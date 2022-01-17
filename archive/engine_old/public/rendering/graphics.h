#pragma once

#include "gfx_instance.h"

class Graphics final
{
    friend class IEngineInterface;

  public:
    Graphics() = delete;

    template <typename GfxContext_T = GfxInterface> static GfxContext_T* get()
    {
        return static_cast<GfxContext_T*>(get_internal());
    }

  private:
    static void create(GfxInterface* instance, const WindowParameters& window_parameters);
    static void destroy();

    static GfxInterface* get_internal();
};
