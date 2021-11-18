# This file must be included by add_subdirectory() from parent, it doesn't work as standalone
cmake_minimum_required(VERSION 3.12)
project(micro-tess-examples)
message(\n===========\n${PROJECT_NAME} \n===========\n)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG  ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE " -fno-rtti -fno-exceptions -fno-unwind-tables \
                            -fstrict-aliasing -O3  -flto \
                            -Wstrict-aliasing -fno-math-errno \
                            -ffast-math -fno-threadsafe-statics \
                            -fdata-sections -ffunction-sections \
                            " CACHE INTERNAL "" FORCE)
#-nostdlib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)
find_package(microgl CONFIG REQUIRED)

if(SDL2_FOUND)
    # micro-tess target should be because this file is included in the parent folder
    set(libs ${SDL2_LIBRARY} microgl::microgl micro-tess)
    set(SOURCES
            example_draw_bezier_patch.cpp
            example_draw_bezier_patch_raw.cpp
            example_draw_path_fill.cpp
            example_draw_path_stroke.cpp
            example_geometry_ear_clipping.cpp
            example_geometry_monotone_triangulation.cpp
            example_geometry_fan_triangulation.cpp
            example_geometry_complex_to_planar_subdivision.cpp
            example_geometry_stroke_tessellation.cpp
            example_geometry_stroke_dash_tessellation.cpp
            example_geometry_elliptic_arc_divider.cpp
            example_geometry_bezier_curve_divider.cpp
            )

    set(SOURCES_SHARED
            libs/stb_image/stb_image.cpp
            src/Resources.cpp
            )

    foreach( testsourcefile ${SOURCES} )
        # I used a simple string replace, to cut off .cpp.
        string( REPLACE ".cpp" "" testname ${testsourcefile} )
        include_directories("${SDL2_INCLUDE_DIR}")
        include_directories("${PROJECT_SOURCE_DIR}/src")
        include_directories("${PROJECT_SOURCE_DIR}/libs/stb_image")
        include_directories("${PROJECT_BINARY_DIR}")
        include_directories("${PROJECT_SOURCE_DIR}/libs/stb_image")
#        include_directories("/usr/local/include/microgl/../")

        add_executable( ${testname} ${testsourcefile} ${SOURCES_SHARED} )
        #        add_executable( ${testname} ${testsourcefile}  )
        # Make sure YourLib is linked to each app
        target_link_libraries( ${testname} ${libs} )
        # this is redundant, as target file dir is always delegated to CMAKE_RUNTIME_OUTPUT_DIRECTORY
        add_custom_command(TARGET ${testname} POST_BUILD COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:${testname}>/assets)
    endforeach( testsourcefile ${SOURCES} )

endif()

# newer clion does not make the binary executable location the current working dir,
# so I copy the assets also to the top level build directory.
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})