#include "engine_examples.h"

#include <assets/asset_texture.h>
#include <assets/asset_material.h>
#include <assets/asset_material_instance.h>
#include <rendering/shaders/shader_property.h>


namespace EngineExamples
{
void create_texture()
{
    // create a texture from data : (pixels, res_x, res_y, channel_count)
    AssetManager::get()->create<ATexture2D>("demo_texture", std::vector<uint8_t>{255, 255, 255, 255}, 1, 1, 1);
}

void create_material()
{
    // # 1. Create vertex shader stage (source file, stage_information)
    const ShaderInfos vertex_config{
        .shader_stage            = VK_SHADER_STAGE_VERTEX_BIT, // This shader will be used for vertex stage
        .use_view_data_buffer    = true,
        .use_scene_object_buffer = true,
    };
    const TAssetPtr<AShader> vertex_shader = AssetManager::get()->create<AShader>("demo_vertex_shader", "data/shaders/default.vs.glsl", vertex_config);

    // # 2. Create fragment shader stage  (source file, stage_information)
    const ShaderInfos fragment_config{
        .shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT, // This shader will be used for fragment stage
        .textures{
            // An array of textures available in the given material
            TextureProperty{.binding_name = "p_diffuse", .texture = TAssetPtr<ATexture>("default_texture")},
        },
    };
    const TAssetPtr<AShader> fragment_shader = AssetManager::get()->create<AShader>("demo_vertex_shader", "data/shaders/default.fs.glsl", fragment_config, vertex_shader);

    // # 3. Create material base.
    MaterialInfos material_infos{
        .vertex_stage = vertex_shader, .fragment_stage = fragment_shader, .renderer_passes = {"render_scene"}, // Specify in which render pass our material will be used
    };
    const TAssetPtr<AMaterialBase> material_base = AssetManager::get()->create<AMaterialBase>("demo_material_base", material_infos);

    // # 4. Create material instance.
    const TAssetPtr<AMaterialInstance> material_instance = AssetManager::get()->create<AMaterialInstance>("demo_material", material_base);
}

void create_mesh()
{
}
}

