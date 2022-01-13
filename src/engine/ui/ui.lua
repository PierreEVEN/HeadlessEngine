

declare_module("ui", {"graphics", "imgui"}, {"cpputils", })

target("ui")
	set_group("engine_new")
	add_headerfiles("private/imgui/**.h")