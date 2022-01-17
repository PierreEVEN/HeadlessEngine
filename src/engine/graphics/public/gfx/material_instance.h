#pragma once
#include "master_material.h"
#include "sampler.h"

#include <memory>

namespace gfx
{
class Texture;
class Buffer;
class CommandBuffer;

class MaterialInstance
{
  public:
    virtual ~MaterialInstance() = default;
    static std::shared_ptr<MaterialInstance> create(const std::shared_ptr<MasterMaterial>& base);

    [[nodiscard]] const std::shared_ptr<MasterMaterial>& get_base() const
    {
        return base_material;
    }

    [[nodiscard]] const std::vector<RenderPassID>& get_compatible_render_passes() const
    {
        return base_material->get_compatible_render_passes();
    }

    virtual void bind_buffer(const std::string& binding_name, const std::shared_ptr<Buffer>& in_buffer)    = 0;
    virtual void bind_texture(const std::string& binding_name, const std::shared_ptr<Texture>& in_texture) = 0;
    virtual void bind_sampler(const std::string& binding_name, const std::shared_ptr<Sampler>& in_sampler) = 0;
    virtual bool bind_material(CommandBuffer* command_buffer)                                              = 0;

  protected:
    MaterialInstance(std::shared_ptr<MasterMaterial> base) : base_material(std::move(base))
    {
    }

  private:
    std::shared_ptr<MasterMaterial> base_material;
};
} // namespace gfx
