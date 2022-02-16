
#include "family.h"

#include "object_ptr.h"

Family::Family(const FamilySignature& in_signature) : component_count(static_cast<uint32_t>(in_signature.elements.size())), object_count(0), signature(in_signature)
{
    const auto& element_count = in_signature.elements.size();
    components                = static_cast<ComponentVector*>(malloc(sizeof(ComponentVector) * element_count));

    for (size_t i = 0; i < in_signature.elements.size(); ++i)
        new (&components[i]) ComponentVector(in_signature.elements[i]);
}

Family::~Family()
{
    delete[] components;
}

void Family::add_object(ObjectPtr* object)
{
    const auto last_index = object_count;
    realloc(object_count + 1);
    object_map[last_index] = object;
    object->pool_index     = last_index;
    object->object_family  = this;
}

void Family::remove_object(ObjectPtr* object)
{
    const auto deleted_index = object->pool_index;
    const auto last_index    = object_count - 1;

    object_map[last_index]->pool_index = deleted_index;
    // Overwrite removed element with last element
    for (uint32_t i = 0; i < component_count; ++i)
        components[i].move(last_index, deleted_index);
    object_map[deleted_index] = object_map[last_index];

    realloc(object_count - 1);

    object->object_family = nullptr;
    object->pool_index    = 0;
}

Component<ComponentPtr> Family::get_component(ObjectPtr*, Class*)
{
    LOG_FATAL("NIY");
    return {};
}
