#include "backend_nazarashader.h"

#include "Nazara/Shader/ShaderBuilder.hpp"
#include "Nazara/Shader/ShaderLangLexer.hpp"
#include "Nazara/Shader/ShaderLangParser.hpp"
#include "Nazara/Shader/SpirvWriter.hpp"

namespace shader_builder::nazarashader_backend
{
std::vector<uint32_t> NazaraShaderCompiler::build_to_spirv(const std::vector<ShaderBlock>& shader_code, EShaderLanguage source_language, EShaderStage shader_stage)
{
    Nz::ShaderLang::Parser             parser;
    Nz::ShaderWriter::States           states;
    Nz::SpirvWriter::Environment       env;
    Nz::SpirvWriter                    writer;
    std::vector<uint32_t>              spv;
    Nz::ShaderAst::StatementPtr        shaderAst;
    std::vector<Nz::ShaderLang::Token> tokens;

        std::string code;
        for (const auto& block : shader_code)
        {
            code += block.text;
        }
        LOG_DEBUG("A : \n%s", code.c_str());
        tokens    = Nz::ShaderLang::Tokenize(std::string_view(code.c_str(), code.size()));
        shaderAst = parser.Parse(tokens);

        LOG_DEBUG("B");
        writer.SetEnv(env);

        LOG_DEBUG("C");
        spv = writer.Generate(*shaderAst, states);
        LOG_DEBUG("success");
        return spv;
}
} // namespace shader_builder::nazarashader_backend