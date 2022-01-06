
target("data")
	set_kind("binary")
	add_headerfiles("shaders/**.shb")
	add_headerfiles("shaders/**.cginc")
	add_headerfiles("shaders/**.cg")
	add_headerfiles("shaders/**.hlsl")
	add_headerfiles("shaders/**.glsl")