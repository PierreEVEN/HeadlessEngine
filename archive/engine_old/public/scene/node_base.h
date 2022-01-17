#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "misc/Frustum.h"

#include <memory>
#include <string>

namespace glm
{
typedef glm::vec<3, double, highp> dvec3;
}

class Scene;

class NodeBase
{
    friend class Scene;

  public:
    NodeBase()
    {
    }
    virtual ~NodeBase() = default;

    [[nodiscard]] Scene* get_render_scene() const
    {
        return render_scene;
    }

    virtual void tick(const double delta_second)
    {
    }

    [[nodiscard]] const glm::dvec3& get_world_position() const
    {
        return world_position;
    }
    [[nodiscard]] const glm::dquat& get_world_rotation() const
    {
        return world_rotation;
    }
    [[nodiscard]] const glm::dvec3& get_world_scale() const
    {
        return world_scale;
    }

    [[nodiscard]] const glm::dvec3& get_relative_position() const
    {
        return rel_position;
    }
    [[nodiscard]] const glm::dquat& get_relative_rotation() const
    {
        return rel_rotation;
    }
    [[nodiscard]] const glm::dvec3& get_relative_scale() const
    {
        return rel_scale;
    }

    [[nodiscard]] const glm::dmat4& get_relative_transform() const
    {
        return rel_transform;
    }

    [[nodiscard]] const glm::dmat4& get_world_transform() const
    {
        return world_transform;
    }

    [[nodiscard]] glm::dvec3 get_forward_vector() const
    {
        return get_world_rotation() * glm::dvec3(1, 0, 0);
    }

    [[nodiscard]] glm::dvec3 get_right_vector() const
    {
        return get_world_rotation() * glm::dvec3(0, 1, 0);
    }

    [[nodiscard]] glm::dvec3 get_up_vector() const
    {
        return get_world_rotation() * glm::dvec3(0, 0, 1);
    }

    void set_world_position(const glm::dvec3& in_position);
    void set_world_rotation(const glm::dquat& in_rotation);
    void set_world_scale(const glm::dvec3& in_scale);

    void set_relative_position(const glm::dvec3& in_position);
    void set_relative_rotation(const glm::dquat& in_rotation);
    void set_relative_scale(const glm::dvec3& in_scale);

    virtual void attach_to(const std::shared_ptr<NodeBase>& new_parent_node, bool b_keep_world_transform = false);
    virtual void detach(bool b_keep_world_transform);

    [[nodiscard]] NodeBase* get_parent() const
    {
        return parent;
    }
    [[nodiscard]] const std::vector<NodeBase*>& get_children() const
    {
        return children;
    }

    [[nodiscard]] std::string get_name() const
    {
        return node_name;
    }

    [[nodiscard]] const Box3D& get_world_bounds() const
    {
        return world_bounds;
    }

  protected:
    void recompute_transform();

    virtual Box3D get_local_bounds()
    {
        return Box3D{};
    }

  private:
    [[nodiscard]] bool ensure_node_can_be_attached(const NodeBase* new_parent) const;
    [[nodiscard]] bool is_node_in_hierarchy(const NodeBase* in_node) const;

    void initialize_internal(Scene* in_scene, NodeBase* in_parent);

    glm::dvec3 rel_position    = glm::dvec3(0.0);
    glm::dquat rel_rotation    = glm::dquat();
    glm::dvec3 rel_scale       = glm::dvec3(1.0);
    glm::dmat4 rel_transform   = glm::dmat4(1.0);
    glm::dvec3 world_position  = glm::dvec3(0.0);
    glm::dquat world_rotation  = glm::dquat();
    glm::dvec3 world_scale     = glm::dvec3(1.0);
    glm::dmat4 world_transform = glm::dmat4(1.0);

    NodeBase*              parent   = nullptr;
    std::vector<NodeBase*> children = {};

    Box3D world_bounds;

    // Initialized in Scene constructor
    Scene*      render_scene;
    std::string node_name;
};