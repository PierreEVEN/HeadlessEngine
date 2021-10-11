include(../src/fetch_prebuilt_lib.cmake)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

	download_prebuilt_lib_dynamic(
		"glfw"
		"https://github.com/glfw/glfw/releases/download/3.3.4/glfw-3.3.4.bin.WIN64.zip"
		"glfw-3.3.4.bin.WIN64/include"
		"glfw-3.3.4.bin.WIN64/lib-vc2019/glfw3dll.lib"
		"glfw-3.3.4.bin.WIN64/lib-vc2019/glfw3.dll"
	)

	download_prebuilt_lib_dynamic(
		"assimp"
		"https://raw.githubusercontent.com/PierreEVEN/PierreEVEN.github.io/master/libs/assimp_winx64.zip"
		"assimp_winx64/include"
		"assimp_winx64/bin/assimp-vc142-mt.lib"
		"assimp_winx64/bin/assimp-vc142-mt.dll"
	)

	#download_prebuilt_lib_dynamic(
	#	"SPIRV-Cross_lib"
	#	"https://raw.githubusercontent.com/PierreEVEN/PierreEVEN.github.io/master/libs/SPIRV-Cross_winx64.zip"
	#	"SPIRV-Cross/include"
	#	"SPIRV-Cross/bin/spirv-cross-c-shared.lib"
	#	"SPIRV-Cross/bin/spirv-cross-c-shared.dll"
	#)

	#download_prebuilt_lib_static(
	#	"glslang_lib"
	#	"https://github.com/KhronosGroup/glslang/releases/download/master-tot/glslang-master-windows-x64-Release.zip"
	#	"include"
	#	"lib/GenericCodeGen.lib;lib/glslang.lib;lib/glslang-default-resource-limits.lib;lib/HLSL.lib;lib/MachineIndependent.lib;lib/OGLCompiler.lib;lib/OSDependent.lib;lib/SPIRV.lib;lib/SPIRV-Tools.lib;lib/SPIRV-Tools-opt.lib;lib/SPVRemapper.lib"
	#)
endif()