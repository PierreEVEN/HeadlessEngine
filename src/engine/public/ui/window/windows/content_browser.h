#pragma once
#include "ui/window/window_base.h"

class ContentBrowser : public WindowBase
{
  public:
    ContentBrowser()
    {
    }

  protected:
    void draw_content() override;
};
