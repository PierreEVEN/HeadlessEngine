
package("directxshadercompilerfixed")

    set_homepage("https://github.com/microsoft/DirectXShaderCompiler/")
    set_description("DirectX Shader Compiler")
    set_license("LLVM")

	
	if is_plat("windows") then	
		add_versions("1.5.2010", "b691f63778f470ebeb94874426779b2f60685fc8711adf1b1f9f01535d9b67f8")
		add_versions("1.6.2104", "ee5e96d58134957443ded04be132e2e19240c534d7602e3ab8fd5adc5156014a")
		add_versions("1.6.2106", "053b2d90c227cae84e7ce636bc4f7c25acd224c31c11a324885acbf5dd8b7aac")		
		local date = {["1.5.2010"] = "2020_10-22",
					  ["1.6.2104"] = "2021_04-20",
					  ["1.6.2106"] = "2021_07_01"}
		add_urls("https://github.com/microsoft/DirectXShaderCompiler/releases/download/v$(version).zip", {version = function (version) return version .. "/dxc_" .. date[tostring(version)] end})		
		on_install("windows|x64", function (package)
			os.cp("bin/x64/*", package:installdir("bin"))
			os.cp("inc/*", package:installdir("include"))
			os.cp("lib/x64/*", package:installdir("lib"))
			package:addenv("PATH", "bin")
		end)
	else	
		add_deps("cmake")
		add_urls("https://github.com/microsoft/DirectXShaderCompiler.git")
		add_versions("1.6.2112", "770ac0cc1d2085c946a1af25ce5e6a1ed083325f")		
		on_install(function (package)
			local configs = {}
			table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
			table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
			import("package.tools.cmake").install(package, configs)
		end)		
    end
package_end()
