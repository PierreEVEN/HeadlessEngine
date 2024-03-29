
#include "scene_importer.h"
#include "mesh_importer.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cpputils/logger.hpp>

#include <assets/asset_material.h>
#include <assets/asset_material_instance.h>
#include <assets/asset_texture.h>
#include <scene/node_base.h>
#include <scene/node_mesh.h>
#include <scene/scene.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::shared_ptr<NodeBase> SceneImporter::process_node(const aiScene* scene, aiNode* ai_node, const std::shared_ptr<NodeBase>& parent, Scene* context_scene)
{
    auto base_node = create_node(scene, ai_node, parent, context_scene);

    for (const auto& ai_child : std::vector<aiNode*>(ai_node->mChildren, ai_node->mChildren + ai_node->mNumChildren))
    {
        auto container_node = process_node(scene, ai_child, base_node, context_scene);
    }

    return base_node;
}

std::shared_ptr<NodeBase> SceneImporter::create_node(const aiScene* scene, aiNode* context, const std::shared_ptr<NodeBase>& parent, Scene* context_scene)
{
    // Extract transformation
    aiVector3t<float> ai_scale;
    aiVector3t<float> ai_pos;
    aiQuaternion      ai_rot;
    context->mTransformation.Decompose(ai_scale, ai_rot, ai_pos);
    const glm::dvec3 position(ai_pos.x, ai_pos.y, ai_pos.z);
    const glm::dquat rotation(ai_rot.w, ai_rot.x, ai_rot.y, ai_rot.z);
    const glm::dvec3 scale(ai_scale.x, ai_scale.y, ai_scale.z);

    auto node = context_scene->add_node<NodeBase>(context->mName.C_Str());
    node->set_relative_position(position);
    node->set_relative_rotation(rotation);
    node->set_relative_scale(scale);
    if (parent)
        node->attach_to(parent);

    for (size_t i = 0; i < context->mNumMeshes; ++i)
    {
        auto mesh_node =
            context_scene->add_node<NMesh>(std::string(context->mName.C_Str()) + "_" + std::to_string(i), meshes_refs[context->mMeshes[i]], material_refs[scene->mMeshes[context->mMeshes[i]]->mMaterialIndex]);
        mesh_node->attach_to(node);
    }

    return node;
}

void SceneImporter::create_default_resources()
{
    // Gltf Shader
    {
        const ShaderInfos vertex_infos{
            .shader_stage            = VK_SHADER_STAGE_VERTEX_BIT,
            .use_view_data_buffer    = true,
            .use_scene_object_buffer = true,
        };
        const auto vertex_shader = AssetManager::get()->create<AShader>("gltf_vertex_shader", "data/shaders/gltf.vs.glsl", vertex_infos);

        const ShaderInfos fragment_infos{
            .shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .textures{
                TextureProperty{.binding_name = "diffuse_color", .texture = TAssetPtr<ATexture>("default_texture")},
            },
        };

        const auto fragment_shader = AssetManager::get()->create<AShader>("gltf_fragment_shader", "data/shaders/gltf.fs.glsl", fragment_infos, vertex_shader);

        MaterialInfos material_infos{
            .vertex_stage    = vertex_shader,
            .fragment_stage  = fragment_shader,
            .renderer_passes = {"render_scene"},
        };

        AssetManager::get()->create<AMaterialBase>("gltf_base_material", material_infos);
    }
}

std::shared_ptr<NodeBase> SceneImporter::import_file(const std::filesystem::path& source_file, const std::string& asset_name, Scene* context_scene)
{
    BEGIN_NAMED_RECORD(IMPORT_SCENE_DATA);
    if (!exists(source_file) || !is_regular_file(source_file))
    {
        LOG_ERROR("file %s doens't exists", source_file.string().c_str());
        return nullptr;
    }
    const aiScene* scene = importer->ReadFile(source_file.string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs);

    if (!scene)
    {
        LOG_ERROR("failed to import scene file %s : %s", source_file.string().c_str(), importer->GetErrorString());
        return nullptr;
    }

    if (object_name.empty())
        object_name = source_file.filename().string().c_str();

    // Generate resources
    texture_refs.resize(scene->mNumTextures);
    for (size_t i = 0; i < scene->mNumTextures; ++i)
        texture_refs[i] = process_texture(scene->mTextures[i], i);

    material_refs.resize(scene->mNumMaterials);
    for (size_t i = 0; i < scene->mNumMaterials; ++i)
        material_refs[i] = process_material(scene, scene->mMaterials[i], i);

    meshes_refs.resize(scene->mNumMeshes);
    for (size_t i = 0; i < scene->mNumMeshes; ++i)
        meshes_refs[i] = MeshImporter::process_mesh(AssetManager::get()->find_valid_asset_id(asset_name + "_" + scene->mMeshes[i]->mName.C_Str()), scene->mMeshes[i], i);

    auto root_node = process_node(scene, scene->mRootNode, nullptr, context_scene);
    return root_node;
}

TAssetPtr<ATexture2D> SceneImporter::process_texture(aiTexture* texture, size_t id)
{
    int      width    = static_cast<int>(texture->mWidth);
    int      height   = static_cast<int>(texture->mHeight);
    int      channels = 4;
    uint8_t* data     = nullptr;
    if (height == 0)
    {
        data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(texture->pcData), texture->mWidth, &width, &height, &channels, channels);
    }
    else
    {
        size_t pixel_count = texture->mWidth * texture->mHeight;
        data               = new uint8_t[pixel_count * channels];
        for (size_t i = 0; i < pixel_count; ++i)
        {
            data[i * channels] = texture->pcData[i].r;
            if (channels > 1)
                data[i * channels + 1] = texture->pcData[i].g;
            if (channels > 2)
                data[i * channels + 2] = texture->pcData[i].b;
            if (channels > 3)
                data[i * channels + 3] = texture->pcData[i].a;
        }
    }

    const auto asset_id = AssetManager::get()->find_valid_asset_id(object_name + "_texture_" + std::string(texture->mFilename.C_Str()));
    auto       text     = AssetManager::get()->create<ATexture2D>(asset_id, std::vector<uint8_t>(data, data + width * height * 4), width, height, channels);

    return text;
}

TAssetPtr<AMaterialInstance> SceneImporter::process_material(const aiScene* scene, aiMaterial* material, size_t id)
{
    aiColor3D diffuse_color;
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);

    int diffuse_index = -1;

    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString texture_path;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path);
        const aiTexture* texture_ptr = scene->GetEmbeddedTexture(texture_path.C_Str());

        for (unsigned int i = 0; i < scene->mNumTextures; i++)
        {
            if (texture_ptr == scene->mTextures[i])
            {
                diffuse_index = i;
                break;
            }
        }
    }

    LOG_DEBUG("using diffuse %d", diffuse_index);

    const auto instance_id       = AssetManager::get()->find_valid_asset_id(object_name + "_material_instance_" + std::string(material->GetName().C_Str()));
    auto       material_instance = AssetManager::get()->create<AMaterialInstance>(instance_id, TAssetPtr<AMaterialBase>("gltf_base_material"));

    if (diffuse_index >= 0)
        material_instance->set_texture("diffuse_color", static_cast<ATexture*>(texture_refs[diffuse_index].get()));

    return material_instance;
}