
#include "ecs/ecs_new.h"

Family* Ecs_New::find_family(const FamilySignature& signature)
{
    const auto& family = families.find(signature);
    if (family == families.end())
    {
        Family* new_family  = new Family(signature);
        families[signature] = new_family;
        return new_family;
    }
    return family->second;
}

Object Ecs_New::new_object()
{
    return this;
}
