add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

set_project("HeadlessEngine")
set_allowedmodes("debug", "release")
set_defaultmode("release")
set_warnings("allextra")
set_allowedarchs("windows|x64")
set_optimize("none") -- aggressive
set_languages("clatest", "cxx20")
set_rundir(".")

--set_symbols("debug", "hidden")

DEBUG = false;


set_runtimes(is_mode("debug") and "MTd" or "MT")
		
add_repositories("third_party deps", {rootdir = os.scriptdir()})
add_requires("glslang", "glfw", "glm", "vulkan-hpp", "vulkan-loader", "vulkan-validationlayers", "vulkan-memory-allocator", "spirv-cross", "imgui docking", "imguizmo", "assimp", "stb")
add_requires("cpputils")

function declare_module(module_name, deps, packages, is_executable)
	if DEBUG then
		print("### "..module_name.." ###")	
	end
	target(module_name)
		if is_executable then
			set_kind("binary")
		else
			set_kind("static")
		end
		add_includedirs("private", {public = false})
		add_includedirs("public", {public = true})
		for _, ext in ipairs({".c", ".cpp"}) do
			add_files("private/**.cpp")
		end
		
		for _, ext in ipairs({".h", ".hpp", ".inl", ".natvis"}) do
			add_headerfiles("public/(**"..ext..")")
			add_headerfiles("private/(**"..ext..")")
		end
		
		if deps then
			if DEBUG then
				print(table.unpack({"\t-- dependencies", table.unpack(deps)}))
			end
			add_deps(table.unpack(deps))
		end
		if packages then
			if DEBUG then
				print(table.unpack({"\t-- packages :", table.unpack(packages)}))
			end
			add_packages(table.unpack(packages))
		end
	target_end()		
end

add_defines("ENABLE_VALIDATION_LAYER")

if DEBUG then
	print("################ building modules ################")
end
includes("src/**.lua");