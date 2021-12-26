#pragma once
#include "framegraph_resource.h"

#include <memory>

namespace gfx
{
class FrameGraph
{
  public:
    FrameGraph()
    {
    }

    void set_root(std::unique_ptr<FrameGraphResource>&& in_root)
    {
        // root = std::move(in_root);
    }

    void generate()
    {
        /*
        if (root)
            root->generate();
            */
    }

    void render()
    {
        root->render();
    }

  private:
    std::unique_ptr<FrameGraphResource> root;
};
} // namespace gfx
