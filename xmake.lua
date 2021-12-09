add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

set_project("HeadlessEngine")
set_defaultmode("release")
set_warnings("allextra")
set_optimize("none")
set_languages("clatest", "cxx20")
set_rundir(".")

function declare_module(module_name, deps, packages, is_executable)
	print("### "..module_name.." ###")	
	target(module_name)
		if is_executable then
			set_kind("binary")
		else
			set_kind("static")
		end
		add_includedirs("public", {public = true})
		add_files("private/**.cpp")
		
		for _, ext in ipairs({".h", ".hpp", ".inl", ".natvis"}) do
			add_headerfiles("public/(**"..ext..")")
		end
		
		if deps then
			print(table.unpack({"\t-- dependencies", table.unpack(deps)}))
			add_deps(table.unpack(deps))
		end
		if packages then
			print(table.unpack({"\t-- packages :", table.unpack(packages)}))
			add_packages(table.unpack(packages))
		end
end

add_repositories("third_party deps", {rootdir = os.scriptdir()})
add_requires("cpputils")

print("################ building modules ################")
includes("src/**.lua");