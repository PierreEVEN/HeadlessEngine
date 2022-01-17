#pragma once
#include <memory>
#include <string>

namespace gfx
{

struct SamplerOptions
{
};

class Sampler
{
  public:
    static std::shared_ptr<Sampler> create(const std::string& sampler_name, const SamplerOptions& options);
    virtual ~Sampler() = default;

  protected:
    Sampler(const SamplerOptions& options) : sampler_options(options)
    {
    }

  private:
    const SamplerOptions sampler_options;
};
} // namespace gfx
