#include "scene/SubScene.h"

#include "gfx/command_buffer.h"

namespace scene
{

void universe_declaration_test()
{
    Universe universe;

    // Add planets
    const auto earth = universe.make_empty_actor();
    const auto mars  = universe.make_empty_actor();

    // Add the ability to move planets
    earth->add_component<Transform>();
    mars->add_component<Transform>();

    const auto earth_scene = earth->add_component<SubScene>();
    const auto mars_scene  = mars->add_component<SubScene>();

    // Add a space ship to earth environment
    const auto my_ship = earth_scene->make_empty_actor();
    my_ship->add_component<Transform>();
    const auto ship_scene = my_ship->add_component<SubScene>(); // The ship is a subscene itself

    // Add thruster to the ship
    const auto& thruster = ship_scene->make_empty_actor();
    thruster->add_component<Transform>();

    // Our ship moved near mars, so we need to make it change current scene
    my_ship->move_to(mars_scene);

    // game loop
    do
    {
        gfx::CommandBuffer* cmd = nullptr;
        universe.tick();
        universe.pre_render();
        universe.render(cmd);
    } while (false);
}
} // namespace scene