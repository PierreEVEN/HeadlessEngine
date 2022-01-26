#pragma once
#include "gfx/primitives.h"

#include <gfx/gfx.h>

class PlanetRenderer
{
  public:
    struct PlanetVertex
    {
        glm::vec3 pos;
    };

    void rectangle_mesh(std::vector<PlanetVertex>& vertices, std::vector<uint32_t>& indices, uint32_t subdivisions, double width, const glm::dvec3& offset)
    {
        const uint32_t subd_vert = subdivisions + 1;

        vertices.resize(subd_vert * subd_vert);
        indices.resize(subdivisions * subdivisions * 6);

        for (uint32_t x = 0; x < subd_vert; ++x)
        {
            for (uint32_t y = 0; y < subd_vert; ++y)
            {
                vertices[x + y * subd_vert] = PlanetVertex{
                    .pos = glm::dvec3((x - subd_vert / 2.0) * width, (y - subd_vert / 2.0) * width, 0) + offset,
                };
            }
        }

        for (uint32_t x = 0; x < subdivisions; ++x)
        {
            for (uint32_t y = 0; y < subdivisions; ++y)
            {
                uint32_t offset = (x + y * subdivisions) * 6;

                indices[offset]     = x + y * subd_vert;
                indices[offset + 2] = x + 1 + y * subd_vert;
                indices[offset + 1] = x + 1 + (y + 1) * subd_vert;

                indices[offset + 4] = x + y * subd_vert;
                indices[offset + 3] = x + 1 + (y + 1) * subd_vert;
                indices[offset + 5] = x + (y + 1) * subd_vert;
            }
        }
    }

    std::shared_ptr<gfx::Mesh> generate_mesh(double size, uint32_t subdivision)
    {
        std::vector<PlanetVertex> vertices;
        std::vector<uint32_t>     indices;
        rectangle_mesh(vertices, indices, subdivision, size, glm::dvec3(0));
        return std::make_shared<gfx::Mesh>("test mesh", vertices, indices, gfx::EBufferType::IMMUTABLE);
    }

    struct ChunkDrawInfos
    {
        double     width;
        glm::dvec3 offset;
    };

    PlanetRenderer(float width, std::shared_ptr<gfx::MasterMaterial> planet_mat)
    {
        const uint32_t subd = 2;

        planet_material = gfx::MaterialInstance::create(planet_mat);
        draw_list       = gfx::Buffer::create("planet chunk buffer", 1, sizeof(ChunkDrawInfos), gfx::EBufferUsage::GPU_MEMORY, gfx::EBufferAccess::CPU_TO_GPU, gfx::EBufferType::IMMEDIATE);
        planet_material->bind_buffer("chunk_buffer", draw_list);
        test_mesh = generate_mesh(1, subd);
    }

    ~PlanetRenderer()
    {
    }

    void tick()
    {
    }

    uint32_t new_draw_count;

    void pre_render(gfx::View* view)
    {
        int scales = 3;

        new_draw_count          = 4 + pow(8, scales);
        const double base_width = 1;

        draw_list->resize(new_draw_count);
        draw_list->set_data(
            [&](void* data)
            {
                auto* chunk_infos     = static_cast<ChunkDrawInfos*>(data);
                chunk_infos[0].width  = base_width;
                chunk_infos[0].offset = glm::dvec3(base_width / 2, base_width / 2, 0);
                chunk_infos[1].width  = base_width;
                chunk_infos[1].offset = glm::dvec3(-base_width / 2, base_width / 2, 0);
                chunk_infos[2].width  = base_width;
                chunk_infos[2].offset = glm::dvec3(-base_width / 2, -base_width / 2, 0);
                chunk_infos[3].width  = base_width;
                chunk_infos[3].offset = glm::dvec3(base_width / 2, -base_width / 2, 0);

                for (int i = 0; i < scales; ++i)
                {
                    double n_width = base_width * pow(2, i + 1);

                    chunk_infos[i * 8].width      = n_width;
                    chunk_infos[i * 8].offset     = glm::dvec3(n_width / 2, n_width / 2, 0);
                    chunk_infos[i * 8 + 1].width  = n_width;
                    chunk_infos[i * 8 + 1].offset = glm::dvec3(n_width / 2, 0, 0);
                    chunk_infos[i * 8 + 2].width  = n_width;
                    chunk_infos[i * 8 + 2].offset = glm::dvec3(n_width / 2, -n_width / 2, 0);

                    chunk_infos[i * 8 + 3].width  = n_width;
                    chunk_infos[i * 8 + 3].offset = glm::dvec3(-n_width / 2, n_width / 2, 0);
                    chunk_infos[i * 8 + 4].width  = n_width;
                    chunk_infos[i * 8 + 4].offset = glm::dvec3(-n_width / 2, 0, 0);
                    chunk_infos[i * 8 + 5].width  = n_width;
                    chunk_infos[i * 8 + 5].offset = glm::dvec3(-n_width / 2, -n_width / 2, 0);

                    chunk_infos[i * 8 + 6].width  = n_width;
                    chunk_infos[i * 8 + 6].offset = glm::dvec3(0, n_width / 2, 0);
                    chunk_infos[i * 8 + 7].width  = n_width;
                    chunk_infos[i * 8 + 7].offset = glm::dvec3(0, -n_width / 2, 0);
                }
            });
    }

    void render(gfx::View* view, gfx::CommandBuffer* command_buffer)
    {
        command_buffer->draw_mesh(test_mesh.get(), planet_material.get(), new_draw_count, 0);
    }

  private:
    std::shared_ptr<gfx::Buffer>           draw_list;
    std::shared_ptr<gfx::Mesh>             test_mesh;
    std::shared_ptr<gfx::MaterialInstance> planet_material;
};
