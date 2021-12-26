#pragma once

#include "box.h"

#include <glm/glm.hpp>

namespace gfx
{

class Bound
{
  public:
    Bound(const Box& box_bounds) : box_init(true), sphere_init(false), box(box_bounds)
    {
    }
    Bound(const Sphere& sphere_bounds) : sphere(sphere_bounds), box_init(false), sphere_init(true)
    {
    }
    Bound() : box_init(false), sphere_init(false)
    {
    }

    const Sphere& get_sphere()
    {
        if (!sphere_init)
        {
            if (box_init)
            {

                sphere = box.sphere();
            }
            else
            {
                sphere = Sphere();
            }
            sphere_init = true;
        }
        return sphere;
    }

    const Box& get_box()
    {
        if (!box_init)
        {
            box = Box(get_sphere());

            box_init = true;
        }

        return box;
    }

    void set_box(const Box& box_bounds)
    {
        box         = box_bounds;
        box_init = true;
        sphere_init = false;
    }

    void set_sphere(const Sphere& sphere_bounds)
    {
        sphere   = sphere_bounds;
        box_init = false;
        sphere_init = true;
    }

  private:
    bool box_init;
    bool sphere_init;

    Sphere sphere;
    Box    box;
};

} // namespace gfx