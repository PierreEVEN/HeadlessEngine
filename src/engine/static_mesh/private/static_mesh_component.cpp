
#include "static_mesh/static_mesh_component.h"

#include "gfx/command_buffer.h"
#include "gfx/view.h"
#include "scene/transform.h"
#include "types/frustum.h"

namespace gfx
{
struct DrawCall
{
    glm::dmat4        matrix;
    Mesh*             mesh;
    MaterialInstance* material;
    bool              operator()(const DrawCall& a, const DrawCall& b) const
    {
        if (a.material < b.material)
            return true;
        if (b.material < a.material)
            return false;

        if (a.mesh < b.mesh)
            return true;
        if (b.mesh < a.mesh)
            return false;

        return false;
    }
};

static std::vector<DrawCall>   drawcalls;
static uint32_t                last_frame_calls = 0;
static std::shared_ptr<Buffer> matrix_buffer;

void StaticMeshComponent::add_systems(ecs::SystemFactory* factory)
{

    factory->pre_render<scene::Transform, StaticMeshComponent>(
        [](ecs::TSystemIterable<scene::Transform, StaticMeshComponent> iterator, View* view)
        {
            drawcalls.reserve(last_frame_calls);
            last_frame_calls = 0;
            for (auto [entity, transform, sm_component] : iterator)
            {
                if (view->get_view_frustum().intersect(sm_component.get_world_bounds()))
                {
                    drawcalls.emplace_back(DrawCall{
                        .matrix   = transform.get_world_transform(),
                        .mesh     = sm_component.mesh.get(),
                        .material = sm_component.material.get(),
                    });
                    last_frame_calls++;
                }
            }

            std::sort(drawcalls.begin(), drawcalls.end());

            matrix_buffer->resize(drawcalls.size());
            matrix_buffer->set_data(
                [](void* data)
                {
                    auto* matrix_data = static_cast<glm::dmat4*>(data);
                    for (int i = 0; i < drawcalls.size(); ++i)
                    {
                        matrix_data[i] = drawcalls[i].matrix;
                    }
                });
        });

    ecs::singleton().on_render.add_lambda(
        []([[maybe_unused]] CommandBuffer* command_buffer)
        {
            if (drawcalls.empty())
                return;
            uint32_t          first         = 0;
            uint32_t          count         = 0;
            Mesh*             last_mesh     = drawcalls[0].mesh;
            MaterialInstance* last_material = drawcalls[0].material;

            for (const auto& draw : drawcalls)
            {
                count++;
                if (last_mesh != draw.mesh || last_material != draw.material)
                {
                    if (last_material != draw.material)
                    {
                        last_material->bind_buffer("matrix_buffer", matrix_buffer);
                    }
                    command_buffer->draw_mesh(draw.mesh, draw.material, count, first);
                    last_mesh     = draw.mesh;
                    last_material = draw.material;
                    first += count;
                    count = 0;
                }
            }
            if (count != 0)
                command_buffer->draw_mesh(last_mesh, last_material, count, first);
        });
}

void StaticMeshComponent::init_system()
{
    matrix_buffer = Buffer::create("sm_matrix_buffer", 1, sizeof(glm::dmat4), EBufferUsage::GPU_MEMORY, EBufferAccess::CPU_TO_GPU, EBufferType::IMMEDIATE);
}

void StaticMeshComponent::destroy_system()
{
    matrix_buffer = nullptr;
}
} // namespace gfx