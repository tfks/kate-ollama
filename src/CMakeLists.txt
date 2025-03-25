add_definitions(-DTRANSLATION_DOMAIN=\"kateollama\")

set(kateollama_SRCS
    kateollamaplugin.cpp
    kateollamaview.cpp
)

add_library(kateollama MODULE ${kateollama_SRCS})

target_link_libraries(kateollama
    KF5::TextEditor
    KF5::I18n
)

install(TARGETS kateollama DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)
