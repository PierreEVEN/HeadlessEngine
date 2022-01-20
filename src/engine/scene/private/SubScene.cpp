
#include "scene/SubScene.h"

#include "gfx/command_buffer.h"

namespace scene
{
class Planet
{
  public:
    static void add_systems(ecs::SystemFactory* factory)
    {
        factory->tick<Transform, Planet>(
            [](ecs::TSystemIterable<Transform, Planet> iterator)
            {
                for (auto [actor, transform, planet] : iterator)
                {
                    planet.velocity += glm::dvec3(1, 0, 0);
                }
            });
    }

  private:
    glm::dvec3 velocity = {};
};

void universe_declaration_test()
{
    Universe universe;

    // Add planets
    const auto earth = universe.new_actor();
    earth->add_component<Transform>();
    earth->add_component<Planet>();
    earth->add_component<SubScene>();

    const auto& mars = earth->duplicate();
    
    // Add a space ship to earth environment
    const auto my_ship = earth->get_component<SubScene>()->new_actor();
    my_ship->add_component<Transform>();
    const auto ship_scene = my_ship->add_component<SubScene>(); // The ship is a subscene itself

    // Add thruster to the ship
    const auto& thruster = ship_scene->new_actor();
    thruster->add_component<Transform>();

    // Our ship moved near mars, so we need to make it change current scene
    my_ship->move_to(&*mars->get_component<SubScene>());

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