/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef OLLAMATOOLWINDOW_HEADER_H
#define OLLAMATOOLWINDOW_HEADER_H

#include <KTextEditor/MainWindow>

#include <KXMLGUIClient>
#include <QTextBrowser>

// #include "plugin.h"

class OllamaToolWindow : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    // Constructor
    explicit OllamaToolWindow(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainwindow);

    // Destructor
    virtual ~OllamaToolWindow();

    void onViewChanged(KTextEditor::View *v);

private:
    KTextEditor::MainWindow *m_mainWindow = nullptr;
    std::unique_ptr<QWidget> m_toolview;
    QTextBrowser *m_previewer = nullptr;
};
#endif // OLLAMATOOLWINDOW_HEADER_H
