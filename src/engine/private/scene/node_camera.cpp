

#include "scene/node_camera.h"

#include "assets/asset_base.h"
#include "assets/asset_shader.h"
#include "assets/asset_shader_buffer.h"
#include "misc/Frustum.h"
#include "scene/scene.h"

#define SSBO_REALLOC_COUNT 10000

struct CameraData
{
    glm::mat4 world_projection = glm::mat4(1.0);
    glm::mat4 view_matrix      = glm::mat4(1.0);
    glm::vec3 camera_location  = glm::vec3(0, 0, 0);
};

NCamera::NCamera() : NodeBase()
{
    debug_draws           = std::make_unique<DebugDraw>(this);
    camera_uniform_buffer = AssetManager::get()->create<AShaderBuffer>("global_camera_uniform_buffer", CameraData{}, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    global_model_ssbo     = AssetManager::get()->create<AShaderBuffer>("global_object_buffer", 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

glm::dmat4 NCamera::get_view_matrix() const
{
    return lookAt(get_world_position(), get_world_position() + get_forward_vector(), get_up_vector());
}

void NCamera::update_view(SwapchainStatus& render_context)
{
    render_context.view = this;
    // UPDATE CAMERA DATA
    world_projection       = glm::perspective<double>(static_cast<double>(get_field_of_view()), render_context.res_x / static_cast<double>(render_context.res_y), static_cast<double>(get_near_clip_plane()),
                                                static_cast<double>(get_far_clip_plane()));
    glm::dmat4 view_matrix = get_view_matrix();
    camera_uniform_buffer->set_data(CameraData{
        .world_projection = world_projection,
        .view_matrix      = view_matrix,
        .camera_location  = get_world_position(),
    });

    Frustum frustum(world_projection * view_matrix);

    get_render_scene()->get_scene_proxy().initialize_buffer(&frustum);

    // UPDATE MODEL MATRICES
    const size_t component_count = get_render_scene()->get_scene_proxy().get_component_count();
    if (global_model_ssbo->get_buffer_size() < component_count * sizeof(glm::mat4))
    {
        global_model_ssbo->resize_buffer(((component_count + SSBO_REALLOC_COUNT) * sizeof(glm::mat4)));
    }
    get_render_scene()->get_scene_proxy().build_transformations(dynamic_cast<AShaderBuffer*>(global_model_ssbo.get()));

    // DRAW MODELS
    get_render_scene()->get_scene_proxy().render(render_context);

    // DRAW DEBUG LINES
    debug_draws->render_wireframe(render_context);
}
