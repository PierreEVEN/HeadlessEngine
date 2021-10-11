#pragma once
#include "camera_basic_controller.h"
#include "engine_interface.h"
#include "scene/scene.h"
#include <ui/imgui/imgui_impl_vulkan.h>

class MainGameInterface final : public IEngineInterface
{
  public:
    RendererConfiguration get_default_render_pass_configuration() override;

  protected:
    void engine_load_resources() override;
    void engine_unload_resources() override;
    void engine_tick(double delta_time) override;

    // Override default graphic interface to implement custom rendering features
    GfxInterface* create_graphic_interface() override;

  private:
    std::unique_ptr<CameraBasicController> controller;
    std::unique_ptr<Scene>                 root_scene;
    std::shared_ptr<NCamera>               main_camera;
    std::unique_ptr<ImGuiImplementation>   imgui_instance;
};
