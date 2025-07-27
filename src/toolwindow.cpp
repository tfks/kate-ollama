/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include <KLocalizedString>
#include <KPluginFactory>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <QHBoxLayout>
#include <QIcon>
#include <QObject>

#include "toolwindow.h"

OllamaToolWindow::OllamaToolWindow(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainwindow)
    : m_mainWindow(mainwindow)
{
    m_toolview.reset(
        m_mainWindow->createToolView(plugin, // pointer to plugin
                                     "markdownpreview", // just an identifier for the toolview
                                     KTextEditor::MainWindow::Right, // we want to create a toolview on the right side
                                     QIcon::fromTheme("preview"), // icon,
                                     i18n("Markdown Preview"))); // User visible name of the toolview, i18n means it will be available for translation

    m_previewer = new QTextBrowser(m_toolview.get());
    // Add the preview widget to our toolview
    m_toolview->layout()->addWidget(m_previewer);
}

OllamaToolWindow::~OllamaToolWindow()
{
}

void OllamaToolWindow::onViewChanged(KTextEditor::View *v)
{
    if (!v || !v->document()) {
        return;
    }

    if (v->document()->highlightingMode().toLower() == "markdown") {
        m_previewer->setMarkdown(v->document()->text());
    }
}
