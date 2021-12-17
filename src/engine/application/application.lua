

declare_module("application", {}, {"cpputils"})

target("application")
	set_group("engine_new")
	
if APPLICATION_BACKEND == "WIN32" then
target("application")
    add_links("ole32", "Shcore", "User32")
else
target("application")
	del_files("private/win32/**.cpp")
end