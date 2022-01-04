#include "shader_builder/parser/parser.h"

#include <cpputils/logger.hpp>

#include "custom_includer.h"
#include "shader_builder/shader_builder.h"

namespace shader_builder::parser
{

class ShaderFileIterator
{
  public:
    ShaderFileIterator(const std::string& shader_file) : line_count(1), ptr(0), shader(shader_file)
    {
    }

    operator bool() const
    {
        return ptr < static_cast<int64_t>(shader.size());
    }

    void operator++()
    {
        if (*this == '\n')
            line_count++;
        ptr += 1;
    }

    char operator+(int n) const
    {
        if (ptr + n >= 0 && ptr + n < static_cast<int64_t>(shader.size()))
            return shader[ptr + n];
        return '\0';
    }

    char operator-(int n) const
    {
        return operator+(-n);
    }

    char operator*() const
    {
        if (ptr >= 0 && ptr < static_cast<int64_t>(shader.size()))
            return shader[ptr];
        return '\0';
    }

    void operator+=(int64_t offset)
    {
        if (offset > 0)
        {
            for (int64_t i = 0; i < offset; ++i)
            {
                if (*this == '\n')
                    line_count++;
                ++ptr;
            }
        }
        else if (offset < 0)
        {
            for (int64_t i = 0; i > offset; --i)
            {
                if (*this == '\n')
                    line_count--;
                --ptr;
            }
        }
    }

    bool operator==(char other) const
    {
        return other == **this;
    }

    bool operator!=(char other) const
    {
        return other != **this;
    }

    [[nodiscard]] uint32_t get_line() const
    {
        return static_cast<uint32_t>(line_count);
    }

  private:
    int64_t            line_count;
    int64_t            ptr;
    const std::string& shader;
};

static bool is_void(char chr)
{
    return chr == ' ' || chr == '\t' || chr == '\n' || chr == '\r';
}

static bool find_string(ShaderFileIterator& it, const char* str)
{
    size_t offset = 0;
    while (it && offset < strlen(str))
    {
        if (it != str[offset])
            return false;
        ++it;
        offset++;
    }
    return offset == strlen(str);
}

static std::string get_line(ShaderFileIterator& it)
{
    std::string line;
    while (it && *it != '\n')
    {
        line += *it;
        ++it;
    }
    return line;
}

static void skip_commentaries(ShaderFileIterator& it)
{
    if (it == '/' && it + 1 == '/')
        get_line(it);
    if (it == '/' && it + 1 == '*')
    {
        while (it && !(it == '*' && it + 1 == '/'))
            ++it;
        it += 2;
    }
}

static std::string get_next_definition(ShaderFileIterator& it)
{
    std::string line;
    while (it && !(it == '[' || (it == '=' && it + 1 == '>')))
    {
        skip_commentaries(it);
        line += *it;
        ++it;
    }
    return line;
}

bool property_trim_func(char chr)
{
    return chr == ';' || chr == '=' || chr == '\t' || chr == '\n' || chr == '\r' || chr == ' ' || chr == '\'' || chr == '"' || chr == ',';
}
static Chunk get_next_chunk(ShaderFileIterator& it)
{
    int64_t indentation = 0;
    int64_t started     = false;
    Chunk   chunk;
    bool    is_init = false;

    while (it && (!started || indentation > 0))
    {
        skip_commentaries(it);

        if (it == '=' && it + 1 == '>')
        {
            it += 2;
            started          = true;
            std::string file = stringutils::trim(get_line(it), property_trim_func);
            chunk.line_start = 0;
            chunk.content    = get()->get_includer()->get_include_data(file);
            chunk.file       = file;
            continue;
        }

        else if (it == '[')
        {
            indentation++;
            started = true;
            ++it;
        }
        else if (it == ']')
        {
            indentation--;
            ++it;
        }
        else
        {
            if (!started && !is_void(*it))
            {
                LOG_ERROR("expected \n[\ngot\n%s", get_line(it).c_str());
                break;
            }

            if (!is_init)
            {
                is_init          = true;
                chunk.line_start = it.get_line();
            }

            chunk.content += *it;
        }

        ++it;
    }
    if (indentation != 0)
        LOG_ERROR("chunk doesn't end correctly :\n%s", chunk.content.c_str());
    return chunk;
}


std::vector<std::pair<std::string, std::string>> parse_head(const std::string& head)
{
    std::vector<std::pair<std::string, std::string>> fields;
    const auto&                                      head_lines = stringutils::split(head, {';'});
    for (int64_t i = 0; i < static_cast<int64_t>(head_lines.size()) - 1; ++i)
    {
        const auto& prop_field = stringutils::split(head_lines[i], {'='});
        if (prop_field.size() != 2)
        {
            LOG_ERROR("syntax error : %s", head_lines[i].c_str());
            continue;
        }

        fields.emplace_back(std::pair(stringutils::trim(prop_field[0], property_trim_func), stringutils::trim(prop_field[1], property_trim_func)));
    }
    return fields;
}

std::vector<std::string> parse_chunk_head(const std::string& chunk_head)
{
    std::vector<std::string> passes;
    for (const auto& field : stringutils::split(chunk_head, {','}))
        passes.emplace_back(stringutils::trim(field, property_trim_func));
    return passes;
}

Result parser::parse_shader(const std::string& shader_file)
{
    Result result;

    std::vector<Chunk> globals;

    ShaderFileIterator it(shader_file);
    while (it)
    {
        skip_commentaries(it);

        if (find_string(it, "#pragma"))
        {
            std::string pragma_directive = get_line(it);
            const auto& fields = stringutils::split(pragma_directive, {' ', '\t'});
            result.properties.insert({fields[0], fields.size() < 2 ? "" : fields[1]});
        }

        if (find_string(it, "head"))
        {
            Chunk head_code = get_next_chunk(it);
            for (const auto& field : parse_head(head_code.content))
                result.default_values.insert(field);
        }

        if (find_string(it, "global"))
        {
            std::string global_args = get_next_definition(it);
            Chunk       global_code = get_next_chunk(it);
            globals.emplace_back(global_code);
        }

        if (find_string(it, "vertex"))
        {
            std::string vertex_args = get_next_definition(it);
            Chunk       vertex_code = get_next_chunk(it);
            for (const auto& pass : parse_chunk_head(vertex_args))
                result.passes[pass].vertex_chunks.emplace_back(vertex_code);
        }

        if (find_string(it, "fragment"))
        {
            std::string fragment_args = get_next_definition(it);
            Chunk       fragment_code = get_next_chunk(it);
            for (const auto& pass : parse_chunk_head(fragment_args))
                result.passes[pass].fragment_chunks.emplace_back(fragment_code);
        }
        ++it;
    }

    for (auto& pass : result.passes)
    {
        for (const auto& global : globals)
        {
            pass.second.fragment_chunks.insert(pass.second.fragment_chunks.begin(), global);
            pass.second.vertex_chunks.insert(pass.second.vertex_chunks.begin(), global);
        }
    }

    return result;
}

} // namespace shader_builder::parser
