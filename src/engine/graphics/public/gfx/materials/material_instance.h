#pragma once
#include "master_material.h"

#include <memory>

namespace gfx
{
class Texture;
class Buffer;
class CommandBuffer;

class MaterialInstance
{
  public:
    static std::shared_ptr<MaterialInstance> create(const std::shared_ptr<MasterMaterial>& base);

    [[nodiscard]] const std::shared_ptr<MasterMaterial>& get_base() const
    {
        return base_material;
    }

    [[nodiscard]] const std::vector<RenderPassID>& get_compatible_render_passes() const
    {
        return base_material->get_compatible_render_passes();
    }

    virtual void bind_buffer(const std::string& binding_name, const std::shared_ptr<Buffer>& in_buffer) = 0;
    virtual void bind_texture(const std::string& binding_name, const std::shared_ptr<Texture>& in_texture)
    {
    }
    virtual void bind_material(CommandBuffer* command_buffer) = 0;

  protected:
    MaterialInstance(const std::shared_ptr<MasterMaterial>& base) : base_material(base)
    {
    }

  private:
    std::shared_ptr<MasterMaterial> base_material;
};
} // namespace gfx
