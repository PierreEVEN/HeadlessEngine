
#include "scene/node_base.h"

#include <algorithm>
#include <cpputils/logger.hpp>

void NodeBase::set_world_position(const glm::dvec3& in_position)
{
    if (parent)
        rel_position = inverse(parent->get_world_transform()) * glm::dvec4(in_position, 1.0);
    else
        rel_position = in_position;
    recompute_transform();
}

void NodeBase::set_world_rotation(const glm::dquat& in_rotation)
{
    if (parent)
        rel_rotation = inverse(parent->get_world_transform()) * mat4_cast(in_rotation); //@TODO : ensure set world rotation works
    else
        rel_rotation = in_rotation;
    recompute_transform();
}

void NodeBase::set_world_scale(const glm::dvec3& in_scale)
{
    if (parent)
        rel_scale = glm::dvec3(1, 1, 1) / parent->get_world_scale() * in_scale; //@TODO : ensure set world scale works
    else
        rel_scale = in_scale;

    recompute_transform();
}

void NodeBase::set_relative_position(const glm::dvec3& in_position)
{
    rel_position = in_position;
    recompute_transform();
}

void NodeBase::set_relative_rotation(const glm::dquat& in_rotation)
{
    rel_rotation = in_rotation;
    recompute_transform();
}

void NodeBase::set_relative_scale(const glm::dvec3& in_scale)
{
    rel_scale = in_scale;
    recompute_transform();
}

void NodeBase::attach_to(const std::shared_ptr<NodeBase>& new_parent_node, const bool b_keep_world_transform)
{
    if (!ensure_node_can_be_attached(new_parent_node.get()))
    {
        LOG_WARNING("cannot attach_to component to this one");
        return;
    }

    new_parent_node->children.emplace_back(this);
    parent = new_parent_node.get();

    if (b_keep_world_transform)
    {
        //@TODO handle b_keep_world_transform
        LOG_FATAL("keep world transform is not handled yet");
    }
    recompute_transform();
}

void NodeBase::detach(bool b_keep_world_transform)
{
    if (b_keep_world_transform)
    {
        //@TODO handle b_keep_world_transform
        LOG_FATAL("keep world transform is not handled yet");
    }
    recompute_transform();

    if (parent)
    {
        parent->children.erase(std::ranges::find(parent->children, this));
    }

    parent = nullptr;
}

bool NodeBase::ensure_node_can_be_attached(const NodeBase* new_parent) const
{
    if (!new_parent)
    {
        LOG_WARNING("invalid node");
    }

    if (new_parent == this)
    {
        LOG_ERROR("cannot attach node to itself");
        return false;
    }

    if (new_parent->render_scene != render_scene)
    {
        LOG_ERROR("cannot attach_to node to this one if it is not owned by the same scene");
        return false;
    }

    if (is_node_in_hierarchy(new_parent))
    {
        LOG_ERROR("cannot attach node %s to node %s : this node is already attached to the current hierarchy", get_name().c_str(), new_parent->get_name().c_str());
        return false;
    }

    if (new_parent->is_node_in_hierarchy(this))
    {
        LOG_ERROR("cannot attach_to node to this one : this node is already attached to the current hierarchy.");
        return false;
    }

    return true;
}

bool NodeBase::is_node_in_hierarchy(const NodeBase* in_node) const
{
    if (in_node == this)
        return true;

    if (parent)
        return parent->is_node_in_hierarchy(in_node);

    return false;
}

void NodeBase::recompute_transform()
{
    rel_transform = glm::translate(glm::dmat4(1.0), rel_position) * glm::mat4_cast(rel_rotation) * glm::scale(glm::dmat4(1), rel_scale);

    if (parent)
    {
        world_transform = parent->get_world_transform() * rel_transform;
        world_position  = parent->get_world_transform() * glm::dvec4(rel_position, 1.0);
        world_rotation  = parent->get_world_rotation() * rel_rotation;
        world_scale     = parent->get_world_scale() * rel_scale;
    }
    else
    {
        world_transform = rel_transform;
        world_position  = rel_position;
        world_rotation  = rel_rotation;
        world_scale     = rel_scale;
    }

    world_bounds = Box3D(get_local_bounds(), get_world_transform());

    for (const auto& child : children)
    {
        child->recompute_transform();
    }
}
