/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef MAINTAB_H
#define MAINTAB_H

// KF Headers
#include <KLocalizedString>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Message>
#include <KXMLGUIClient>

#include <QComboBox>
#include <QObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QWidget>

#include "ollamasystem.h"
#include "plugin.h"
#include "toolwidget.h"

class MainTab : public QWidget, public KXMLGUIClient
{
    Q_OBJECT

public:
    MainTab(KTextEditor::MainWindow *mainWindow, OllamaToolWidget *parent = nullptr);
    ~MainTab();

private:
    void loadModels();
    void onFullPrompt();

    KTextEditor::MainWindow *m_mainWindow = nullptr;
    QComboBox *m_modelsComboBox;
    QPlainTextEdit *m_textAreaOutput;
    QPlainTextEdit *m_textAreaInput;
    QPushButton *m_newTabBtn;

    KateOllamaPlugin *m_plugin;
    OllamaSystem *m_ollamaSystem;
};
#endif // MAINTAB_H
