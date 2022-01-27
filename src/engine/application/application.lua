

declare_module("application", {"types"}, {"cpputils"})

target("application")
	set_group("engine_new")
	
if APPLICATION_BACKEND == "WIN32" then
target("application")
    add_links("ole32", "Shcore", "User32", { public = true })
else
target("application")
	del_files("private/win32/**.cpp")
end