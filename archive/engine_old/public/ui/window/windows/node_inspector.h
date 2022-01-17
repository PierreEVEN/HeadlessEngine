#pragma once
#include "ui/window/window_base.h"

class NCamera;
class NodeBase;
class Scene;

class NodeInspector : public WindowBase
{
  public:
    NodeInspector(NodeBase* in_node, NCamera* in_camera);

    void set_node(NodeBase* in_node);

  protected:
    void draw_content() override;

    NodeBase* node;
    NCamera*  camera;
};
