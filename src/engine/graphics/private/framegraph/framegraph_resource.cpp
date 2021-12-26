
#include "gfx/framegraph/framegraph_resource.h"

namespace gfx
{
void FrameGraphResource::add_child(const std::shared_ptr<FrameGraphResource>& render_pass)
{
    render_pass->parents.emplace_back(this);
    children.emplace_back(render_pass);
}
} // namespace gfx
