#pragma once

#include <string>

namespace sail::commands {

int cmd_new(const std::string& name, const std::string& path = ".");

} // namespace sail::commands