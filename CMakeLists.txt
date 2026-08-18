cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)
project(${_name} VERSION ${_version} LANGUAGES C CXX)

option(ENABLE_FRONTEND_API "Use obs-frontend-api" ON)
option(ENABLE_QT "Use Qt" ON)
option(WG_BUILD_INSTALLER "Enable CPack/NSIS packaging" ON)

include(compilerconfig)
include(defaults)
include(helpers)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${CMAKE_PROJECT_NAME} MODULE)
find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
endif()


# -----------------------------------------------------------------------------
# Native PSD/PSB parsing for Worship Graphics.
# OpenPSD is pinned to an exact commit and built statically into the plugin.
# We deliberately avoid OpenPSD's own top-level CMakeLists so its GTK demo/tests
# are not pulled into the OBS plugin build.
# -----------------------------------------------------------------------------
FetchContent_Declare(
  wg_openpsd_source
  GIT_REPOSITORY https://github.com/warrengalyen/OpenPSD.git
  GIT_TAG 2151a8b92fe6a7cd98cea4f9ec50a7e5df4fb921
  GIT_SHALLOW FALSE
  SOURCE_SUBDIR _worship_graphics_no_subproject
)
FetchContent_MakeAvailable(wg_openpsd_source)

add_library(wg_openpsd STATIC
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_parse.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_stream.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_endian.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_context.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_alloc.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_rle.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_layer_decode.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_descriptor.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_unicode.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_zip.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_render.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_text_layer.c
  ${wg_openpsd_source_SOURCE_DIR}/src/psd_text_layer_parse.c
)
set_target_properties(wg_openpsd PROPERTIES
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
  POSITION_INDEPENDENT_CODE ON
)
if(MSVC)
  target_compile_options(wg_openpsd PRIVATE /TC)
endif()
target_include_directories(wg_openpsd PUBLIC
  ${wg_openpsd_source_SOURCE_DIR}/include
)

find_package(ZLIB QUIET)
if(ZLIB_FOUND)
  target_compile_definitions(wg_openpsd PRIVATE PSD_ENABLE_ZIP)
  target_link_libraries(wg_openpsd PRIVATE ZLIB::ZLIB)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE wg_openpsd)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/app-state.cpp src/app-state.hpp
  src/model.hpp
  src/graphics-renderer.cpp src/graphics-renderer.hpp
  src/output-source.cpp src/output-source.hpp
  src/main-dock.cpp src/main-dock.hpp
  src/operator-page.cpp src/operator-page.hpp
  src/design-page.cpp src/design-page.hpp
  src/psd-importer.cpp src/psd-importer.hpp
  src/design-canvas-widget.cpp src/design-canvas-widget.hpp
  src/theme.cpp src/theme.hpp
  src/template-factory.cpp src/template-factory.hpp
  src/template-library.cpp src/template-library.hpp
  src/timeline-widget.cpp src/timeline-widget.hpp
  src/bible-engine.cpp src/bible-engine.hpp
  src/module-stubs.hpp
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE WG_VERSION="${PROJECT_VERSION}")
if(MSVC)
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4 /permissive-)
endif()

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

# Direct OBS runtime layout for our standalone NSIS installer.
if(WIN32)
  install(TARGETS ${CMAKE_PROJECT_NAME}
    RUNTIME DESTINATION "obs-plugins/64bit"
    LIBRARY DESTINATION "obs-plugins/64bit"
  )
endif()
install(DIRECTORY data/ DESTINATION "data/obs-plugins/${_name}")

if(WG_BUILD_INSTALLER AND WIN32)
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_PACKAGE_NAME "Worship Graphics")
  set(CPACK_PACKAGE_VENDOR "Worship Graphics")
  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
  set(CPACK_PACKAGE_FILE_NAME "WorshipGraphics-Setup-${PROJECT_VERSION}")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "obs-studio")
  set(CPACK_NSIS_DISPLAY_NAME "Worship Graphics for OBS")
  set(CPACK_NSIS_PACKAGE_NAME "Worship Graphics")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  include(CPack)
endif()
