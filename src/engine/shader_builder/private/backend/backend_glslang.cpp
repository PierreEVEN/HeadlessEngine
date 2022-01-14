#include "backend/backend_glslang.h"

#include "internal.h"
#include "shader_builder/shader_builder.h"

#include "custom_includer.h"

#include <glslang/MachineIndependent/iomapper.h>

#include <glslang/SPIRV/GlslangToSpv.h>

#include <cpputils/logger.hpp>

namespace shader_builder::glslang_backend
{

class IoMapResolver : public glslang::TIoMapResolver
{
  public:
    IoMapResolver(glslang::TProgram& in_program, const InterstageData& in_input, InterstageData& in_output) : program(in_program), input(in_input), output(in_output)
    {
        output.binding_index = in_input.binding_index;
    }

  private:
    std::unordered_map<void*, int> binding_map = {};
    int                            output_id   = 0;
    int                            input_id    = 0;

    bool validateBinding(EShLanguage, glslang::TVarEntryInfo& ent) override
    {
        if (!binding_map.contains(ent.symbol))
            binding_map[ent.symbol] = ++output.binding_index;
        return true;
    }
    int resolveBinding(EShLanguage, glslang::TVarEntryInfo& ent) override
    {
        return ent.newBinding = binding_map[ent.symbol];
    }
    int resolveSet(EShLanguage, glslang::TVarEntryInfo&) override
    {
        return -1;
    }
    int resolveUniformLocation(EShLanguage, glslang::TVarEntryInfo& ent) override
    {
        return ent.newLocation = binding_map[ent.symbol];
    }
    bool validateInOut(EShLanguage, glslang::TVarEntryInfo&) override
    {
        return true;
    }

    int resolveInOutLocation(EShLanguage, glslang::TVarEntryInfo& ent) override
    {
        if (ent.symbol->getQualifier().builtIn)
            return -1;

        const auto names = stringutils::split(ent.symbol->getName().c_str(), {'.'});

        std::string prop_name = "@";
        for (int i = 1; i < names.size(); ++i)
            prop_name += "." + names[i];

        if (ent.symbol->getQualifier().isPipeInput() && input.stage_outputs.contains(prop_name))
        {
            ent.newLocation = input.stage_outputs.find(prop_name)->second;
        }
        else
        {
            if (ent.symbol->getQualifier().isPipeOutput())
            {
                output.stage_outputs[prop_name] = output_id;
                ent.newLocation                 = output_id++;
            }
            else
            {
                ent.newLocation = input_id++;
            }
        }
        return ent.newLocation;
    }
    int resolveInOutComponent(EShLanguage, glslang::TVarEntryInfo&) override
    {
        return -1;
    }
    int resolveInOutIndex(EShLanguage, glslang::TVarEntryInfo&) override
    {
        return -1;
    }
    void notifyBinding(EShLanguage, glslang::TVarEntryInfo&) override
    {
    }
    void notifyInOut(EShLanguage, glslang::TVarEntryInfo&) override
    {
    }
    void beginNotifications(EShLanguage) override
    {
    }
    void endNotifications(EShLanguage) override
    {
    }
    void beginResolve(EShLanguage) override
    {
    }
    void endResolve(EShLanguage) override
    {
    }
    void beginCollect(EShLanguage) override
    {
    }
    void endCollect(EShLanguage) override
    {
    }
    void reserverStorageSlot(glslang::TVarEntryInfo&, TInfoSink&) override
    {
    }
    void reserverResourceSlot(glslang::TVarEntryInfo&, TInfoSink&) override
    {
    }
    void addStage(EShLanguage, glslang::TIntermediate&) override
    {
    }
    glslang::TProgram&    program;
    const InterstageData& input;
    InterstageData&       output;
};

std::vector<uint32_t> GlslangCompiler::build_to_spirv(const std::vector<ShaderBlock>& shader_code, EShaderLanguage source_language, EShaderStage shader_stage, const InterstageData& input_stage_data,
                                                      InterstageData& output_stage_data)
{
    std::vector<uint32_t> compilation_result;

    const EShLanguage stage = shader_stage == EShaderStage::Vertex ? EShLangVertex : EShLangFragment;

    std::vector<const char*> vertex_strings(shader_code.size());
    std::vector<int32_t>     vertex_lengths(shader_code.size());
    std::vector<const char*> vertex_names(shader_code.size());
    for (size_t i = 0; i < shader_code.size(); ++i)
    {
        vertex_strings[i] = shader_code[i].text.c_str();
        vertex_lengths[i] = static_cast<int32_t>(shader_code[i].text.size());
        vertex_names[i]   = shader_code[i].name.c_str();
    }

    const EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgReadHlsl | EShMsgHlslLegalization);

    glslang::TShader shader(stage);
    shader.setEnvInput(source_language == EShaderLanguage::GLSL ? glslang::EShSourceGlsl : glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 0);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    shader.setEntryPoint("main");
    shader.setAutoMapBindings(true);
    shader.setAutoMapLocations(true);
    shader.setHlslIoMapping(true);
    shader.setEnvTargetHlslFunctionality1();
    shader.setStringsWithLengthsAndNames(vertex_strings.data(), vertex_lengths.data(), vertex_names.data(), static_cast<int>(shader_code.size()));

    if (!shader.parse(&get_resources(), 0, false, messages, *get()->get_includer()))
    {
        LOG_FATAL("failed to parse : \n%s", shader.getInfoLog());
        return compilation_result;
    }

    glslang::TProgram program{};
    program.addShader(&shader);
    if (!program.link(messages))
    {
        LOG_FATAL("failed to link : \n%s", shader.getInfoLog());
        return compilation_result;
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spv_option;

    spv_option.generateDebugInfo = true;
    spv_option.disableOptimizer  = false;
    spv_option.optimizeSize      = false;
    spv_option.disassemble       = false;
    spv_option.validate          = true;
    IoMapResolver Resovler(program, input_stage_data, output_stage_data);
    // This step is essential to set bindings and descriptor sets
    program.mapIO(&Resovler);

    GlslangToSpv(*program.getIntermediate(stage), compilation_result, &logger, &spv_option);

    return compilation_result;
}
} // namespace shader_builder::glslang_backend
