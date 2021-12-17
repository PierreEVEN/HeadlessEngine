#pragma once
#include "node_base.h"

class Scene;

class NPrimitive : public NodeBase
{
  public:
    NPrimitive()          = default;
    virtual ~NPrimitive() = default;

    void set_visible(bool b_visible);

  private:
    bool is_visible = false;
};