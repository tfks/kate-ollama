cmake_minimum_required(VERSION 3.10)
project(KateOllama)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )
find_package(KF5TextEditor REQUIRED)
find_package(KF5CoreAddons REQUIRED COMPONENTS)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMSetupVersion)

add_library(kate_ollama_plugin MODULE ollama_plugin.cpp)

target_include_directories(kate_ollama_plugin PRIVATE ${KF5TextEditor_INCLUDE_DIRS})
target_link_libraries(kate_ollama_plugin PRIVATE KF5::TextEditor KF5::CoreAddons Qt6::Widgets)

set_target_properties(kate_ollama_plugin PROPERTIES
    VERSION ${PLUGIN_VERSION}
    SOVERSION ${PLUGIN_VERSION_MAJOR}
    AUTOMOC ON
)

install(TARGETS kate_ollama_plugin LIBRARY DESTINATION ${KDE_INSTALL_PLUGINDIR})

message(STATUS "KDE_INSTALL_PLUGINDIR: ${KDE_INSTALL_PLUGINDIR}")

install(FILES ${CMAKE_SOURCE_DIR}/metadata.json DESTINATION ${KDE_INSTALL_PLUGINDIR})
