
#include "static_mesh/static_mesh_component.h"

#include "gfx/View.h"
#include "gfx/buffer.h"
#include "gfx/command_buffer.h"
#include "types/frustum.h"

namespace gfx
{
struct MeshProxy
{
    glm::dmat4 matrix;
    size_t     material_handle;
    size_t     material_instance_handle;
    size_t     mesh_handle;
    //@TODO add sort support
};

void StaticMeshComponent::add_systems(ecs::SystemFactory* factory)
{
    ecs::singleton().on_render.add_lambda(
        []([[maybe_unused]] gfx::View* view)
        {

        });

    factory->pre_render<StaticMeshComponent>(
        [](ecs::TSystemIterable<StaticMeshComponent> iterator, gfx::View* view)
        {
            std::vector<MeshProxy> render_matrices;

            for (auto [entity, sm_component] : iterator)
            {
                if (view->get_view_frustum().intersect(sm_component.get_bounds()))
                {
                    render_matrices.emplace_back(MeshProxy{});
                }
            }

            std::ranges::sort(render_matrices); //@TODO

            gfx::Buffer test("fazfazf", sizeof(glm::dmat4) * render_matrices.size(), EBufferUsage::GPU_MEMORY, EBufferAccess::CPU_TO_GPU);

            test.set_data(
                [&](void* data)
                {
                    for (size_t index = 0; index < render_matrices.size(); ++index)
                        static_cast<glm::dmat4*>(data)[index] = render_matrices[index].matrix;
                });
        });

    factory->render<StaticMeshComponent>(
        [](gfx::CommandBuffer* command_buffer, ecs::TSystemIterable<StaticMeshComponent> iterator)
        {
            for (auto [entity, sm_component] : iterator)
            {
                // render instances
                command_buffer->draw_mesh(nullptr, nullptr, {}, 0);
            }
        });
}
} // namespace gfx