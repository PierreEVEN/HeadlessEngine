#pragma once

#include <filesystem>

#include "assets/asset_ptr.h"
#include "assimp/Importer.hpp"

class Scene;
struct aiNode;
class AMeshData;
class NodeBase;

class AMaterial;
class ATexture;

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

    std::shared_ptr<NodeBase> import_file(const std::filesystem::path& source_file, const std::string& asset_name, Scene* context_scene);

  private:
    TAssetPtr<ATexture>  process_texture(struct aiTexture* texture, size_t id);
    TAssetPtr<AMaterial> process_material(const struct aiScene* scene, struct aiMaterial* material, size_t id);

    std::shared_ptr<NodeBase> process_node(const aiScene* scene, aiNode* ai_node, const std::shared_ptr<NodeBase>& parent, Scene* context_scene);
    std::shared_ptr<NodeBase> create_node(const aiScene* scene, aiNode* context, const std::shared_ptr<NodeBase>& parent, Scene* context_scene);

    std::string                       object_name;
    std::vector<TAssetPtr<ATexture>>  texture_refs;
    std::vector<TAssetPtr<AMaterial>> material_refs;
    std::vector<TAssetPtr<AMeshData>> meshes_refs;

    std::unique_ptr<Assimp::Importer> importer;
};