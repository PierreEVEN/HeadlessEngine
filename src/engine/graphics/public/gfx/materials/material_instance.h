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
    virtual void bind_texture([[maybe_unused]] const std::string& binding_name, [[maybe_unused]] const std::shared_ptr<Texture>& in_texture)
    {
    }
    virtual void bind_material(CommandBuffer* command_buffer) = 0;

    template <typename Data_T> void push_constants(bool is_vertex_stage, const Data_T& data)
    {
        push_constants_internal(is_vertex_stage, &data, sizeof(Data_T));
    }

    // virtual void push_constants_internal(void* data, size_t data_size) = 0;

  protected:
    MaterialInstance(std::shared_ptr<MasterMaterial> base) : base_material(std::move(base))
    {
    }

    virtual void push_constants_internal(bool is_vertex_stage, const void* data, size_t data_size) = 0;

  private:
    std::shared_ptr<MasterMaterial> base_material;
};
} // namespace gfx
