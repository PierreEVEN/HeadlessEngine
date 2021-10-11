#pragma once

#include <filesystem>

#include <assets/asset_ptr.h>
#include <assimp/Importer.hpp>

class AMaterialInstance;
class Scene;
struct aiNode;
class AMeshData;
class NodeBase;

class ATexture2D;

class SceneImporter final
{
  public:
    SceneImporter()
    {
        importer = std::make_unique<Assimp::Importer>();
    }
    ~SceneImporter()
    {
    }

    static void create_default_resources();

    std::shared_ptr<NodeBase> import_file(const std::filesystem::path& source_file, const std::string& asset_name, Scene* context_scene);

  private:
    TAssetPtr<ATexture2D>  process_texture(struct aiTexture* texture, size_t id);
    TAssetPtr<AMaterialInstance> process_material(const struct aiScene* scene, struct aiMaterial* material, size_t id);

    std::shared_ptr<NodeBase> process_node(const aiScene* scene, aiNode* ai_node, const std::shared_ptr<NodeBase>& parent, Scene* context_scene);
    std::shared_ptr<NodeBase> create_node(const aiScene* scene, aiNode* context, const std::shared_ptr<NodeBase>& parent, Scene* context_scene);

    std::string                       object_name;
    std::vector<TAssetPtr<ATexture2D>>  texture_refs;
    std::vector<TAssetPtr<AMaterialInstance>> material_refs;
    std::vector<TAssetPtr<AMeshData>> meshes_refs;

    std::unique_ptr<Assimp::Importer> importer;
};