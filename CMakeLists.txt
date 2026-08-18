cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)
project(${_name} VERSION ${_version} LANGUAGES C CXX)

option(ENABLE_FRONTEND_API "Use obs-frontend-api" ON)
option(ENABLE_QT "Use Qt" ON)
option(WG_BUILD_INSTALLER "Enable CPack/NSIS packaging" ON)

include(compilerconfig)
include(defaults)
include(helpers)

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

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/app-state.cpp src/app-state.hpp
  src/model.hpp
  src/graphics-renderer.cpp src/graphics-renderer.hpp
  src/output-source.cpp src/output-source.hpp
  src/main-dock.cpp src/main-dock.hpp
  src/operator-page.cpp src/operator-page.hpp
  src/design-page.cpp src/design-page.hpp
  src/theme.cpp src/theme.hpp
  src/module-stubs.hpp
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE WG_VERSION="${PROJECT_VERSION}")
if(MSVC)
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4 /permissive-)
endif()

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

# Extra locale/theme data used by the plugin.
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
