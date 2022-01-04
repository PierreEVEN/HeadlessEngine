#pragma once
#include <string>
#include <vector>

namespace shader_builder
{

struct OperationError
{
    uint32_t    column = 0;
    uint32_t    line   = 0;
    std::string error_message;
};

class OperationStatus
{
  public:
    OperationStatus() : is_success(true)
    {
    }
    void add_error(const OperationError& error)
    {
        errors.emplace_back(error);
    }

    operator bool() const
    {
        return is_success;
    }

    [[nodiscard]] const std::vector<OperationError>& get_errors() const
    {
        return errors;
    }

  private:
    std::vector<OperationError> errors;
    bool                  is_success;
};

} // namespace shader_builder