

#include "misc/primitives.h"

std::vector<Vertex> primitive::CubePrimitive::get_vertices() const
{
    return {
        Vertex{
            .pos  = {-100, -100, -100},
            .uv   = {0, 0},
            .norm = {0, 0, -1},
        },
        Vertex{.pos = {100, -100, -100}, .uv = {1, 0}, .norm = {0, 0, -1}},
        Vertex{.pos = {100, 100, -100}, .uv = {1, 1}, .norm = {0, 0, -1}},
        Vertex{.pos = {-100, 100, -100}, .uv = {0, 1}, .norm = {0, 0, -1}},

        Vertex{.pos = {-100, -100, 100}, .uv = {0, 0}, .norm = {0, 0, 1}},
        Vertex{.pos = {100, -100, 100}, .uv = {1, 0}, .norm = {0, 0, 1}},
        Vertex{.pos = {100, 100, 100}, .uv = {1, 1}, .norm = {0, 0, 1}},
        Vertex{.pos = {-100, 100, 100}, .uv = {0, 1}, .norm = {0, 0, 1}},

        Vertex{.pos = {100, -100, 100}, .uv = {0, 0}, .norm = {-1, 0, 0}},
        Vertex{.pos = {100, -100, -100}, .uv = {1, 0}, .norm = {-1, 0, 0}},
        Vertex{.pos = {100, 100, -100}, .uv = {1, 1}, .norm = {-1, 0, 0}},
        Vertex{.pos = {100, 100, 100}, .uv = {0, 1}, .norm = {-1, 0, 0}},

        Vertex{.pos = {-100, -100, 100}, .uv = {0, 0}, .norm = {1, 0, 0}},
        Vertex{.pos = {-100, -100, -100}, .uv = {1, 0}, .norm = {1, 0, 0}},
        Vertex{.pos = {-100, 100, -100}, .uv = {1, 1}, .norm = {1, 0, 0}},
        Vertex{.pos = {-100, 100, 100}, .uv = {0, 1}, .norm = {1, 0, 0}},

        Vertex{.pos = {100, -100, 100}, .uv = {0, 0}, .norm = {0, 1, 0}},
        Vertex{.pos = {100, -100, -100}, .uv = {1, 0}, .norm = {0, 1, 0}},
        Vertex{.pos = {-100, -100, -100}, .uv = {1, 1}, .norm = {0, 1, 0}},
        Vertex{.pos = {-100, -100, 100}, .uv = {0, 1}, .norm = {0, 1, 0}},

        Vertex{.pos = {100, 100, 100}, .uv = {0, 0}, .norm = {0, -1, 0}},
        Vertex{.pos = {100, 100, -100}, .uv = {1, 0}, .norm = {0, -1, 0}},
        Vertex{.pos = {-100, 100, -100}, .uv = {1, 1}, .norm = {0, -1, 0}},
        Vertex{.pos = {-100, 100, 100}, .uv = {0, 1}, .norm = {0, -1, 0}},
    };
}

std::vector<uint32_t> primitive::CubePrimitive::get_indices() const
{
    return {
        0, 2, 1, 0, 3, 2, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 14, 13, 12, 15, 14, 16, 18, 17, 16, 19, 18, 20, 21, 22, 20, 22, 23,
    };
}
