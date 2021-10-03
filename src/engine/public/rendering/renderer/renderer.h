#pragma once

#include "render_pass_description.h"
#include "swapchain.h"
#include <cpputils/logger.hpp>

class RenderPass;
class Swapchain;
class Framebuffer;

class RendererConfiguration final
{
  public:
    RendererConfiguration() = default;
    RendererConfiguration(const std::vector<RenderPassSettings>& render_pass_descriptions) : pass_descriptions(render_pass_descriptions)
    {
    }

    [[nodiscard]] RenderPassSettings* get_render_pass(const std::string& pass_name)
    {
        for (auto& pass : pass_descriptions)
            if (pass.pass_name == pass_name)
                return &pass;
        return nullptr;
    }

    [[nodiscard]] std::vector<RenderPassSettings>& get_pass_descriptions()
    {
        return pass_descriptions;
    }

    [[nodiscard]] std::string to_string() const;

  private:
    std::vector<RenderPassSettings> pass_descriptions;
};

class Renderer
{
  public:
    Renderer(Swapchain* in_swapchain);
    ~Renderer();

    void init(VkExtent2D in_render_resolution);
    void render_frame(SwapchainFrame& swapchain_frame);

    RenderPass*                       get_render_pass(const std::string& pass_name);
    [[nodiscard]] RenderPassSettings* get_render_pass_configuration(const std::string& pass_name)
    {
        return renderer_configuration.get_render_pass(pass_name);
    }

    void set_render_pass_description(RendererConfiguration in_renderer_configuration);

  private:
    Swapchain*                                swapchain = nullptr;
    RendererConfiguration                     renderer_configuration;
    std::vector<std::unique_ptr<RenderPass>>  render_passes;
    std::vector<std::unique_ptr<Framebuffer>> per_pass_framebuffer;
    std::vector<std::vector<VkCommandBuffer>> per_image_pass_command_buffers;

    VkExtent2D render_resolution;
};
