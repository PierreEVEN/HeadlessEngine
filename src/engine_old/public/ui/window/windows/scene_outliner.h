#pragma once
#include "ui/window/window_base.h"

class NCamera;
class NodeInspector;
class NodeBase;
class Scene;

class SceneOutliner : public WindowBase
{
  public:

      SceneOutliner(Scene* in_scene, NCamera* in_camera);
      
  protected:
      void draw_node(NodeBase* node);
    void draw_content() override;
    Scene* scene;
    NodeBase*  selected_node = nullptr;
    NodeInspector* inspector     = nullptr;
    NCamera*        camera;
};
