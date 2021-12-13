

declare_module("application", {"config", "utils"}, {"cpputils"})

if is_plat("windows") then
target("application")
    add_links("ole32", "Shcore")
end