cmake_minimum_required(VERSION 3.16)
project(kate-ollama VERSION 1.0)

find_package(ECM ${KF5_DEP_VERSION} QUIET REQUIRED NO_MODULE)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt${QT_MAJOR_VERSION}Widgets CONFIG REQUIRED)

set(KF5_DEP_VERSION "5.90")
find_package(KF5 ${KF5_DEP_VERSION}
    REQUIRED COMPONENTS
        CoreAddons # Core addons on top of QtCore
        I18n # For localization
        TextEditor # The editor component
)

# This line defines the actual target
kcoreaddons_add_plugin(kate-ollama
    INSTALL_NAMESPACE "ktexteditor")

target_sources(
  kate-ollama
  PRIVATE
  ollama_plugin.h
  ollama_plugin.cpp
  metadata.json
)

# This makes the plugin translatable
target_compile_definitions(kate-ollama PRIVATE TRANSLATION_DOMAIN="kate-ollama")

target_link_libraries(kate-ollama
    PRIVATE
    KF5::CoreAddons KF5::I18n KF5::TextEditor
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
