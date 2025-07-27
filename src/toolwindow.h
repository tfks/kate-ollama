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

class OllamaToolWindow : public QWidget
{
    Q_OBJECT

public:
    // Constructor
    explicit OllamaToolWindow(KTextEditor::MainWindow *mainWindow, QWidget *parent = nullptr);

    // Destructor
    virtual ~OllamaToolWindow();

    void onViewChanged(KTextEditor::View *v);

private:
    KTextEditor::MainWindow *m_mainWindow = nullptr;
};
#endif // OLLAMATOOLWINDOW_HEADER_H
