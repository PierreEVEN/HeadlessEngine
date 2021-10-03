#pragma once
#include "ios/input_manager.h"

#include <memory>

class NCamera;

class CameraBasicController
{
  public:
    CameraBasicController(const std::shared_ptr<NCamera>& in_camera, InputManager* in_input_manager);

  private:
    std::shared_ptr<NCamera> controlled_camera;
    double                  movement_speed = 1000.0;

    void update_camera();

    double pitch = 0.0;
    double yaw   = 0.0;
    InputManager* input_manager;
};