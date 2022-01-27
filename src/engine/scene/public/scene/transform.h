#pragma once

#include <glm/detail/type_quat.hpp>
#include <glm/glm.hpp>

namespace scene
{
class Transform
{
  public:
    Transform() : world_transform(1), local_position(0), local_rotation(glm::quat()), local_scale(1)
    {
    }

    void set_relative_position(const glm::dvec3& position)
    {
        local_position = position;
    }
    void set_relative_rotation(const glm::quat& rotation)
    {
        local_rotation = rotation;
    }
    void set_relative_scale(const glm::vec3& scale)
    {
        local_scale = scale;
    }

    void set_world_position([[maybe_unused]] const glm::dvec3& position)
    {
    }
    void set_world_rotation([[maybe_unused]] const glm::quat& rotation)
    {
    }
    void set_world_scale([[maybe_unused]] const glm::vec3& scale)
    {
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

    [[nodiscard]] const glm::dvec3& get_world_position() const
    {
        return local_position;
    }
    [[nodiscard]] const glm::quat& get_world_rotation() const
    {
        return local_rotation;
    }
    [[nodiscard]] const glm::vec3& get_world_scale() const
    {
        return local_scale;
    }

    [[nodiscard]] glm::dmat4 get_relative_transform() const
    {
        return {};
    }
    [[nodiscard]] const glm::dmat4& get_world_transform() const
    {
        return world_transform;
    }

  private:
    glm::dmat4 world_transform;
    glm::dvec3 local_position;
    glm::quat  local_rotation;
    glm::vec3  local_scale;
};
} // namespace scene