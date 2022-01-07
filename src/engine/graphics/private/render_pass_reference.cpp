#include "gfx/render_pass_reference.h"

#include <cpputils/logger.hpp>
#include <unordered_map>

namespace gfx
{

static std::unordered_map<std::string, uint8_t> pass_id_map;
uint64_t                                        valid_pass_id_bitmap = 0;

RenderPassID RenderPassID::get(const std::string& pass_name)
{
    const auto& id = pass_id_map.find(pass_name);

    if (id == pass_id_map.end())
    {
        LOG_FATAL("there is no render pass named %s", pass_name.c_str());
        return RenderPassID(UINT8_MAX);
    }
    return RenderPassID(id->second);
}

bool RenderPassID::exists(const std::string& pass_name)
{
    return pass_id_map.contains(pass_name);
}

bool RenderPassID::operator==(const RenderPassID& other) const
{
    return other.internal_id == internal_id && other.operator bool() && operator bool();
}

bool RenderPassID::operator!=(const RenderPassID& other) const
{
    return operator==(other);
}

RenderPassID::operator bool() const
{
    return valid_pass_id_bitmap & 1LL << internal_id;
}

std::string RenderPassID::name() const
{
    for (const auto& it : pass_id_map)
    {
        if (it.second == internal_id)
            return it.first;
    }
    return "";
}

uint8_t RenderPassID::get_internal_num() const
{
    return internal_id;
}

RenderPassID RenderPassID::declare(const std::string& pass_name)
{
    if (pass_id_map.contains(pass_name))
        LOG_FATAL("a render pass named %s already exists", pass_name.c_str());

    for (uint8_t pass_id = 0; pass_id < MAX_RENDER_PASS; ++pass_id)
    {
        if (!(valid_pass_id_bitmap & 1LL << pass_id))
        {
            valid_pass_id_bitmap |= 1LL << pass_id;
            pass_id_map[pass_name] = pass_id;
            return RenderPassID(pass_id);
        }
    }
    LOG_ERROR("reached the max allowed pass count. Use MAX_RENDER_PASS to avoid this limit");
    return RenderPassID(UINT8_MAX);
}
} // namespace gfx
