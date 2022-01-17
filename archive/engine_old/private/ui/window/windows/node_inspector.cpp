

#include "ui/window/windows/node_inspector.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "scene/node_base.h"
#include "scene/node_camera.h"

#include <cpputils/simplemacros.hpp>

#if OS_WINDOWS
#include <corecrt_math_defines.h>
#endif

NodeInspector::NodeInspector(NodeBase* in_node, NCamera* in_camera) : node(in_node), camera(in_camera)
{
}

void NodeInspector::set_node(NodeBase* in_node)
{
    node = in_node;
}

glm::dmat4 ToImGuizmo(const glm::dmat4& src)
{
    glm::dmat4 res;
    for (auto row = 0; row < 4; row++)
    {
        for (auto col = 0; col < 4; col++)
            res[row][col] = src[col][row];
    }
    return res;
}

void NodeInspector::draw_content()
{
    if (node)
    {
        ImGui::Text("editing : %s", node->get_name().c_str());
        ImGui::Separator();

        float position[3];
        position[0] = static_cast<float>(node->get_relative_position().x);
        position[1] = static_cast<float>(node->get_relative_position().y);
        position[2] = static_cast<float>(node->get_relative_position().z);

        ImGui::DragFloat3("location", position);

        node->set_relative_position(glm::dvec3(position[0], position[1], position[2]));

        const auto euler = glm::eulerAngles(node->get_relative_rotation());

        float rotation[3];
        rotation[0] = static_cast<float>(euler.x * 180 / M_PI);
        rotation[1] = static_cast<float>(euler.y * 180 / M_PI);
        rotation[2] = static_cast<float>(euler.z * 180 / M_PI);

        ImGui::DragFloat3("rotation", rotation);

        node->set_relative_rotation(glm::dquat(glm::dvec3(rotation[0] * M_PI / 180, rotation[1] * M_PI / 180, rotation[2] * M_PI / 180)));

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::BeginFrame();
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        glm::mat4 view  = camera->get_view_matrix();
        glm::mat4 world = camera->get_world_projection();
        float     object_matrix[16];

        glm::vec3 pos   = node->get_world_position();
        glm::quat rot   = node->get_world_rotation();
        glm::vec3 scale = node->get_world_scale();

        ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.w, &scale.x, object_matrix);
        ImGuizmo::Manipulate(&view[0][0], &world[0][0], ImGuizmo::TRANSLATE, ImGuizmo::WORLD, object_matrix, nullptr, nullptr);
        ImGuizmo::DecomposeMatrixToComponents(object_matrix, &pos.x, &rot.w, &scale.x);

        node->set_world_position(pos);
        // node->set_relative_rotation(rot);
        // node->set_relative_scale(scale);
    }
}
