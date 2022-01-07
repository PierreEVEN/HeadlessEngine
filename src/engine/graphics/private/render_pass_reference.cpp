#include "gfx/render_pass_reference.h"

#include <unordered_map>
#include <cpputils/logger.hpp>

namespace gfx
{

static std::unordered_map<std::string, uint8_t> pass_id_map;

RenderPassID RenderPassID::get(const std::string& pass_name)
{
    const auto& id = pass_id_map.find(pass_name);

    if (id == pass_id_map.end())
    {
        LOG_ERROR("there is no render pass named %s", pass_name.c_str());
        return RenderPassID(0, false);
    }
    return RenderPassID(id->second, true);
}

bool RenderPassID::is_valid_index(uint8_t index)
{
    for (const auto& pass : pass_id_map)
    {
        if (index == pass.second)
            return true;
    }
    return false;
}

std::string RenderPassID::get_name() const
{
    for (const auto& pass : pass_id_map)
    {
        if (id == pass.second)
            return pass.first;
    }
    LOG_FATAL("integrity error");
}

RenderPassID RenderPassID::declare(const std::string& pass_name)
{
    if (pass_id_map.contains(pass_name))
        LOG_FATAL("a render pass named %s already exists", pass_name.c_str());

    uint8_t pass_id = 0;
    for (uint8_t i = 0; i < MAX_RENDER_PASS; ++i)
    {
        bool already_exists = false;
        for (const auto& pass : pass_id_map)
        {
            if (pass.second == pass_id)
            {
                already_exists = true;
                pass_id++;
                break;
            }
        }
        if (!already_exists)
        {
            pass_id_map[pass_name] = pass_id;
            return RenderPassID(pass_id, true);
        }
    }
    LOG_ERROR("reached the max allowed pass count. Use MAX_RENDER_PASS to avoid this limit");
    return RenderPassID(0, false);
}
}
