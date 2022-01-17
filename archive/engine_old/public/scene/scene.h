#pragma once

#include "scene_proxy.h"

#include "assets/asset_shader_buffer.h"
#include <cpputils/logger.hpp>

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class NCamera;
class AShaderBuffer;
class NodeBase;
class NPrimitive;

class Scene
{
    friend class NPrimitive;

  public:
    Scene();

    void tick(const double delta_second);

    template <typename Node_T, typename... Args_T> std::shared_ptr<Node_T> add_node(const std::string& node_name, Args_T&&... arguments)
    {
        Node_T* node_storage       = static_cast<Node_T*>(std::malloc(sizeof(Node_T)));
        node_storage->render_scene = this;

        std::shared_ptr<Node_T> node_ptr(node_storage);

        new (node_storage) Node_T(std::forward<Args_T>(arguments)...);

        if (!node_storage->render_scene)
        {
            LOG_ERROR("don't call NodeBase() constructor in children class : %s", typeid(Node_T).name());
        }
        node_storage->node_name = node_name;

        scene_nodes.emplace_back(std::dynamic_pointer_cast<NodeBase>(node_ptr));

        return node_ptr;
    }

    [[nodiscard]] SceneProxy& get_scene_proxy()
    {
        return scene_proxy;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<NodeBase>>& get_nodes() const
    {
        return scene_nodes;
    }

  private:
    std::vector<std::shared_ptr<NodeBase>> scene_nodes;
    SceneProxy                             scene_proxy;
};
