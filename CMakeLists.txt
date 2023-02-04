cmake_minimum_required (VERSION 3.12)
project (mycelium)

#---------------------------------------------------------------------------------------
# main project
#---------------------------------------------------------------------------------------

# find all source and header files
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.hpp" "src/*.inl")

# organize the files according to the organization in the file system
# from: http:#cmake.3232098.n2.nabble.com/Keep-folder-structure-in-Visual-Studio-Project-generated-with-CMake-td7586044.html
macro(GroupSources curdir)
	file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)
	foreach(child ${children})
		if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			GroupSources(${curdir}/${child})
		else()
				string(REPLACE "/" "\\" groupname ${curdir})
			source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
		endif()
	endforeach()
endmacro()

# execute the macro
GroupSources(src)

add_executable(mycelium ${SOURCE_FILES})
set_target_properties(mycelium PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
)

include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
	set_property(TARGET mycelium PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mycelium)

#---------------------------------------------------------------------------------------
# libraries
#---------------------------------------------------------------------------------------

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/../AcaEngine/" "dependencies")
target_link_libraries(mycelium PUBLIC AcaEngine)
target_include_directories(mycelium PRIVATE "src")