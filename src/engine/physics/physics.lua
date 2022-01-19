

declare_module("physics", {}, {"cpputils", "glm", "bullet3"})

target("physics")
	set_group("engine_new/modules")
	add_headerfiles("private/bullet/**.h")