

#include "scene/node_mesh.h"

#include "assets/asset_material.h"
#include "assets/asset_material_instance.h"
#include "assets/asset_mesh_data.h"
#include "misc/Frustum.h"
#include "scene/scene.h"

struct MeshProxyData
{
    NMesh*             owner          = nullptr;
    AMaterial*         material_base  = nullptr;
    AMaterialInstance* material       = nullptr;
    VkBuffer           vertex_buffer  = VK_NULL_HANDLE;
    VkBuffer           index_buffer   = VK_NULL_HANDLE;
    size_t             index_count    = 0;
    glm::dmat4         mesh_transform = glm::dmat4(1.0);
    size_t             instance_index = 1;
    Box3D              bounds         = {};

    // comparison function
    bool operator==(const MeshProxyData& other) const
    {
        return other.material == material && other.vertex_buffer == vertex_buffer;
    }

    // sort function
    bool operator()(const MeshProxyData& a, const MeshProxyData& b) const
    {
        if (a.material_base < b.material_base)
            return true;
        if (b.material_base < a.material_base)
            return false;

        if (a.material < b.material)
            return true;
        if (b.material < a.material)
            return false;

        if (a.vertex_buffer < b.vertex_buffer)
            return true;
        if (b.vertex_buffer < a.vertex_buffer)
            return false;

        return false;
    }

    // culling
    [[nodiscard]] bool display_test(Frustum* frustum)
    {
        return frustum->is_box_visible(bounds);
    }
};

NMesh::NMesh(TAssetPtr<AMeshData> in_mesh, TAssetPtr<AMaterialInstance> in_material) : mesh(in_mesh), material(in_material)
{
    if (!material)
    {
        LOG_ERROR("material is not valid");
        return;
    }

    proxy_entity_handle = get_render_scene()->get_scene_proxy().add_entity(MeshProxyData{
        .owner          = this,
        .material_base  = dynamic_cast<AMaterial*>((material->get_material_base()).get_const()),
        .material       = dynamic_cast<AMaterialInstance*>(material.get()),
        .vertex_buffer  = mesh->get_vertex_buffer(),
        .index_buffer   = mesh->get_index_buffer(),
        .index_count    = mesh->get_indices_count(),
        .mesh_transform = get_world_transform(),
    });
    recompute_transform();
}

NMesh::~NMesh()
{
    get_render_scene()->get_scene_proxy().remove_entity(proxy_entity_handle);
}

void NMesh::register_component(Scene* target_scene)
{
    target_scene->get_scene_proxy().register_entity_type<MeshProxyData>(
        [](MeshProxyData& entity, SwapchainFrame& render_context, size_t instance_count, size_t first_instance) {
            if (render_context.last_used_material != entity.material)
            {
                render_context.last_used_material = entity.material;
                if (!entity.material)
                    LOG_FATAL("material is not valid");

                entity.material->update_descriptor_sets(render_context.render_pass, render_context.view, render_context.image_index);

                auto* pipeline = entity.material->get_material_base()->get_pipeline(render_context.render_pass);

                vkCmdBindDescriptorSets(render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->get_pipeline_layout(), 0, 1,
                                        &(*entity.material->get_descriptor_sets(render_context.render_pass))[render_context.image_index].descriptor_set, 0, nullptr);

                if (render_context.last_used_material_base != entity.material_base)
                {
                    if (!entity.material_base)
                        LOG_FATAL("material_base is not valid");
                    render_context.last_used_material_base = entity.material_base;

                    auto* pipeline = entity.material->get_material_base()->get_pipeline(render_context.render_pass);

                    vkCmdBindPipeline(render_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());
                }
            }

            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(render_context.command_buffer, 0, 1, &entity.vertex_buffer, offsets);
            vkCmdBindIndexBuffer(render_context.command_buffer, entity.index_buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(render_context.command_buffer, static_cast<uint32_t>(entity.index_count), static_cast<uint32_t>(instance_count), 0, 0, static_cast<uint32_t>(first_instance));
        },
        [](MeshProxyData& entity, AShaderBuffer* buffer_storage, size_t buffer_index) {
            const glm::mat4 temp_transform = entity.mesh_transform;
            buffer_storage->write_buffer(&temp_transform, sizeof(glm::mat4), buffer_index * sizeof(glm::mat4));
        });
}

void NMesh::update_data()
{
    proxy_data_lock.lock();
    *get_render_scene()->get_scene_proxy().find_entity_group<MeshProxyData>()->get_entity(proxy_entity_handle) = MeshProxyData{
        .owner          = this,
        .material_base  = dynamic_cast<AMaterial*>((material->get_material_base()).get_const()),
        .material       = dynamic_cast<AMaterialInstance*>(material.get()),
        .vertex_buffer  = mesh->get_vertex_buffer(),
        .index_buffer   = mesh->get_index_buffer(),
        .index_count    = mesh->get_indices_count(),
        .mesh_transform = get_world_transform(),
        .bounds         = get_world_bounds(),
    };
    proxy_data_lock.unlock();
}

void NMesh::tick(const double delta_second)
{
    update_data();
}

Box3D NMesh::get_local_bounds()
{
    return get_mesh()->get_bounds();
}
