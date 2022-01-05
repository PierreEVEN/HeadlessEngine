#pragma once
#include <string>
#include <vector>

namespace shader_builder
{

struct OperationError
{
    int32_t    column = 0;
    int32_t     line   = 0;
    std::string error_message;
};

class OperationStatus
{
  public:

    void add_error(const OperationError& error)
    {
        errors.emplace_back(error);
    }

    void append(const OperationStatus& other)
    {
        for (const auto& er : other.get_errors())
            errors.emplace_back(er);
    }
    
    operator bool() const
    {
        return errors.empty();
    }

    [[nodiscard]] const std::vector<OperationError>& get_errors() const
    {
        return errors;
    }

  private:
    std::vector<OperationError> errors;
};

} // namespace shader_builder