
function(download_prebuilt_lib P_LIBNAME P_URL)
	set(THIRD_PARTY_DIR "${PROJECT_SOURCE_DIR}/third_party/deps")
	set(LIB_PATH "${THIRD_PARTY_DIR}/${P_LIBNAME}")

	if(IS_DIRECTORY "${THIRD_PARTY_DIR}/${P_LIBNAME}")
		message("-- Found ${P_LIBNAME} binaries.")
	else()
		message("-- ${P_LIBNAME} binaries are missing. Downloading...")
		file(DOWNLOAD
			"${P_URL}"
			"${LIB_PATH}/${P_LIBNAME}.zip"
			SHOW_PROGRESS
			STATUS DOWNLOAD_STATUS
		)
		list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
		list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
		
		if (${STATUS_CODE} EQUAL 0)
			message("-- Downloaded ${P_LIBNAME} binaries")
			file(ARCHIVE_EXTRACT INPUT ${LIB_PATH}/${P_LIBNAME}.zip DESTINATION ${LIB_PATH})
			file(REMOVE ${LIB_PATH}/${P_LIBNAME}.zip)
		else()
			message("-- failed to fetch ${P_LIBNAME} : ${ERROR_MESSAGE}")
			file(REMOVE_RECURSE ${LIB_PATH})
		endif()
	endif()	
	
	if (IS_DIRECTORY ${LIB_PATH})
		set(should_compile_${P_LIBNAME} "OFF" CACHE INTERNAL "should_download")
	else()
		set(should_compile_${P_LIBNAME} "ON" CACHE INTERNAL "should_download")
	endif()
	
endfunction()

function(download_prebuilt_lib_static P_LIBNAME P_URL P_INCUDE_PATH P_LIB_PATH)
	set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party/deps)
	set(LIB_PATH "${THIRD_PARTY_DIR}/${P_LIBNAME}")
	download_prebuilt_lib(${P_LIBNAME} ${P_URL})
	if (NOT should_compile_${P_LIBNAME})
		add_library(${P_LIBNAME} INTERFACE)
		target_include_directories(${P_LIBNAME} INTERFACE ${LIB_PATH}/${P_INCUDE_PATH})
		
		FOREACH(SUBLIB ${P_LIB_PATH})
			message(${LIB_PATH}/${SUBLIB})
			target_link_libraries(${P_LIBNAME} INTERFACE ${LIB_PATH}/${SUBLIB})
		ENDFOREACH()
	endif()
endfunction()

function(download_prebuilt_lib_dynamic P_LIBNAME P_URL P_INCUDE_PATH P_LIB_PATH P_DLL_PATH)
	set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party/deps)
	set(LIB_PATH ${THIRD_PARTY_DIR}/${P_LIBNAME})
	download_prebuilt_lib(${P_LIBNAME} ${P_URL})
	if (NOT should_compile_${P_LIBNAME})
		add_library(${P_LIBNAME} INTERFACE)
		target_include_directories(${P_LIBNAME} INTERFACE ${LIB_PATH}/${P_INCUDE_PATH})
		target_link_libraries(${P_LIBNAME} INTERFACE ${LIB_PATH}/${P_LIB_PATH})
		file(COPY ${LIB_PATH}/${P_DLL_PATH} DESTINATION ${PROJECT_SOURCE_DIR}/temp/dlls/)
	endif()
endfunction()

