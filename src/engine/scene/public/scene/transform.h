#pragma once

#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>

namespace scene
{

class Transform
{
  public:
    Transform() : parent(nullptr), world_transform(1), local_position(0), local_rotation(glm::quat()), local_scale(1)
    {
    }

    void attach_to(Transform* new_parent)
    {
        parent = new_parent;
    }

    [[nodiscard]] glm::dmat4 get_relative_transform() const
    {
        return translate(scale(glm::dmat4(), static_cast<glm::dvec3>(local_scale)), local_position);
    }

    [[nodiscard]] const glm::dmat4& get_world_transform() const
    {
        return world_transform;
    }

    [[nodiscard]] const glm::dvec3& get_relative_position() const
    {
        return local_position;
    }

    [[nodiscard]] const glm::quat& get_relative_rotation() const
    {
        return local_rotation;
    }

    [[nodiscard]] const glm::vec3& get_relative_scale() const
    {
        return local_scale;
    }

  private:
    Transform* parent;
    glm::dmat4 world_transform;
    glm::dvec3 local_position;
    glm::quat  local_rotation;
    glm::vec3  local_scale;
};

} // namespace scene