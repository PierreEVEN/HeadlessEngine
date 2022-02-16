#pragma once
#include "family.h"
#include "types/robin_hood_map.h"

#include "ecs/object.h"

#define NULL_ID 0

using ObjectID = uint64_t;

class Ecs_New
{
  public:
    Family* find_family(const FamilySignature& signature);

    Object new_object();

  private:
    robin_hood::unordered_map<FamilySignature, Family*> families;
};