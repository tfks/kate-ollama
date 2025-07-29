/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef OLLAMATOOLWIDGET_HEADER_H
#define OLLAMATOOLWIDGET_HEADER_H

#include <KTextEditor/MainWindow>

#include <KXMLGUIClient>
#include <QTabWidget>
#include <QTextBrowser>

#include "plugin.h"
#include "src/ollamasystem.h"

class OllamaToolWidget : public QWidget
{
    Q_OBJECT

public:
    // Constructor
    explicit OllamaToolWidget(KateOllamaPlugin *plugin, KTextEditor::MainWindow *mainWindow, OllamaSystem *ollamaSystem, QWidget *parent = nullptr);

    // Destructor
    virtual ~OllamaToolWidget();

    // Add's a new tab
    void newTab();

    void onViewChanged(KTextEditor::View *v);

private:
    KateOllamaPlugin *m_plugin;
    KTextEditor::MainWindow *m_mainWindow = nullptr;
    QTabWidget m_tabWidget;
    OllamaSystem *m_ollamaSystem;
};
#endif // OLLAMATOOLWIDGET_HEADER_H
