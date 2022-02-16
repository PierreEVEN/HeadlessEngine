

declare_module("ecs", {"types", "reflection"}, {"cpputils"})

target("ecs")
	set_group("engine_new")
	add_headerfiles("private/**.h")