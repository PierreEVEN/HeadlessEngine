

#include "camera_basic_controller.h"

#include "ios/input_manager.h"
#include "scene/node_camera.h"

#include <corecrt_math_defines.h>

CameraBasicController::CameraBasicController(const std::shared_ptr<NCamera>& in_camera, InputManager* in_input_manager) : controlled_camera(in_camera), input_manager(in_input_manager)
{
    input_manager->add_action(InputAction("camera_move_forward", {keyboard::key_w}));
    input_manager->add_action(InputAction("camera_move_backward", {keyboard::key_s}));
    input_manager->add_action(InputAction("camera_move_right", {keyboard::key_d}));
    input_manager->add_action(InputAction("camera_move_left", {keyboard::key_a}));
    input_manager->add_action(InputAction("camera_move_up", {keyboard::key_space}));
    input_manager->add_action(InputAction("camera_move_down", {keyboard::key_left_shift}));
    input_manager->add_action(InputAction("camera_move_down", {keyboard::key_left_shift}));

    input_manager->add_action(InputAction("possess_camera", {keyboard::key_escape}));

    input_manager->add_axis(InputAxis("camera_look_right", {mouse::mouse_x}));
    input_manager->add_axis(InputAxis("camera_look_up", {mouse::mouse_y}));

    input_manager->add_axis(InputAxis("camera_speed", {mouse::wheel_y}));

    input_manager->get_action("possess_camera")->pressed_event.add_lambda([&](const InputAction& input_action, const double delta_time) {
        input_manager->set_show_mouse_cursor(!input_manager->show_mouse_cursor());
    });

    input_manager->get_action("camera_move_forward")->press_event.add_lambda([&](const InputAction& input_action, const double delta_time) {
        controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_forward_vector() * movement_speed * delta_time);
    });

    input_manager->get_action("camera_move_backward")->press_event.add_lambda([&](const InputAction& input_action, const double delta_time) {
        controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_forward_vector() * -movement_speed * delta_time);
    });

    input_manager->get_action("camera_move_right")->press_event.add_lambda([&](const InputAction& input_action, const double delta_time) {
        controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_right_vector() * -movement_speed * delta_time);
    });

    input_manager->get_action("camera_move_left")->press_event.add_lambda([&](const InputAction& input_action, const double delta_time) {
        controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_right_vector() * movement_speed * delta_time);
    });

    input_manager->get_action("camera_move_up")->press_event.add_lambda([&](const InputAction& input_action, const double delta_time) {
        controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_up_vector() * movement_speed * delta_time);
    });

    input_manager->get_action("camera_move_down")->press_event.add_lambda([&](const InputAction& input_action, const double delta_time) {
        controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_up_vector() * -movement_speed * delta_time);
    });

    input_manager->get_axis("camera_look_right")->axis_event.add_lambda([&](const InputAxis& input_action, const double value, const double delta_time) {
        if (!input_manager->show_mouse_cursor())
            yaw -= value * 0.005;
        update_camera();
    });

    input_manager->get_axis("camera_look_up")->axis_event.add_lambda([&](const InputAxis& input_action, const double value, const double delta_time) {
        if (!input_manager->show_mouse_cursor())
            pitch += value * 0.005;
        if (pitch > M_PI / 2 - 0.0001)
            pitch = M_PI / 2 - 0.0001;
        if (pitch < -M_PI / 2 + 0.0001)
            pitch = -M_PI / 2 + 0.0001;
        update_camera();
    });

    
    input_manager->get_axis("camera_speed")->axis_event.add_lambda([&](const InputAxis& input_action, const double value, const double delta_time) {
        movement_speed *= value > 0.1 ? 1.5 : value < -0.1 ? 0.75 : 1;
        });
}

void CameraBasicController::update_camera()
{
    controlled_camera->set_relative_rotation(glm::dquat(glm::dvec3(0, pitch, yaw)));
}
