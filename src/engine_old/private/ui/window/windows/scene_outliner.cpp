

#include "ui/window/windows/scene_outliner.h"

#include "imgui.h"
#include "scene/node_base.h"
#include "scene/scene.h"
#include "ui/window/windows/node_inspector.h"

#include "engine_interface.h"

SceneOutliner::SceneOutliner(Scene* in_scene, NCamera* in_camera) : scene(std::move(in_scene)), camera(in_camera)
{
}

size_t node_index;

void SceneOutliner::draw_node(NodeBase* node)
{
    int flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (node->get_children().empty())
        flags |= ImGuiTreeNodeFlags_Leaf;
    if (node == selected_node)
        flags |= ImGuiTreeNodeFlags_Selected;

    const bool b_expand = ImGui::TreeNodeEx((node->get_name() + std::string("##") + std::to_string(node_index++)).c_str(), flags);

    if (ImGui::IsItemClicked(GLFW_MOUSE_BUTTON_1))
    {
        selected_node = node;
        if (!inspector)
            inspector = WindowManager::create<NodeInspector>("node inspector", nullptr, selected_node, camera);
        else
        {
            inspector->set_node(selected_node);
        }
    }

    if (b_expand)
    {
        for (auto* child : node->get_children())
        {
            draw_node(child);
        }
        ImGui::TreePop();
    }
}

void SceneOutliner::draw_content()
{
    node_index = 0;
    if (ImGui::BeginChild("SceneOutlinerList"))
    {
        ImGui::Text("items : %d", scene->get_nodes().size());
        ImGui::Separator();

        for (const auto& node : scene->get_nodes())
        {
            if (!node->get_parent())
                draw_node(node.get());
        }
    }
    ImGui::EndChild();
}
