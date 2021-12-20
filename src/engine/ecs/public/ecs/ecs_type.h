#pragma once
#include <cstdint>


using EcsID           = uint32_t;
using ActorID         = EcsID;

template <typename BaseClass_T> class TypeIdGenerator
{
  public:
    template <class Type_T> [[nodiscard]] static EcsID get()
    {
        static const EcsID class_id = current_id++;
        return class_id;
    }

  private:
    static EcsID current_id;
};

template <class BaseClass_T> EcsID TypeIdGenerator<BaseClass_T>::current_id = 0;
