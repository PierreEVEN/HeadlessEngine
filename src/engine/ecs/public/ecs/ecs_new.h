#pragma once
#include "ecs/object.h"
#include "ecs/family.h"
#include "types/robin_hood_map.h"

class Ecs_New
{
  public:
    Family* find_family(const FamilySignature& signature);

    Object new_object();

  private:
    robin_hood::unordered_map<FamilySignature, Family*> families;
};