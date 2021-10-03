
#include "ui/window/window_base.h"

#include "engine_interface.h"
#include "imgui.h"
#include "rendering/renderer/surface.h"

WindowManager::~WindowManager()
{
    for (auto* window : windows)
    {
        delete window;
    }
}

void WindowManager::draw()
{
    for (int64_t i = windows.size() - 1; i >= 0; --i)
    {
        if (windows[i]->open)
        {
            windows[i]->draw();
        }
        else
        {
            delete windows[i];
            windows.erase(windows.begin() + i);
        }
    }
}

void WindowManager::add_window(WindowBase* window)
{
    for (window->window_id = 0; window_ids.find(window->window_id) != window_ids.end(); ++window->window_id)
        ;

    windows.push_back(window);
}

void WindowManager::remove_window(WindowBase* window)
{
    windows.erase(std::find(windows.begin(), windows.end(), window));
    window_ids.erase(window_ids.find(window->window_id));

    delete window;
}

WindowBase::~WindowBase()
{
    LOG_INFO("close window %s", window_name);
}

void WindowBase::close()
{
    if (open)
    {
        open = false;
        for (auto& child : children)
        {
            child->close();
        }
    }
}

void WindowBase::draw()
{
    if (!open)
        return;

    if (ImGui::Begin((std::string(window_name) + "##" + std::to_string(window_id)).c_str(), &open))
    {
        draw_content();
    }
    ImGui::End();
}

void DemoWindow::draw_content()
{
    ImGui::ShowDemoWindow();
}
