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
#include <QLabel>
#include <QLineEdit>
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
    MainTab(KateOllamaPlugin *plugin, KTextEditor::MainWindow *mainWindow, OllamaSystem *ollamaSystem, OllamaToolWidget *parent = nullptr);
    ~MainTab();

public slots:
    void handleSignalModelsListLoaded(const QList<QJsonValue> &modelsList);

private:
    void loadModels();
    void onFullPrompt();

    KTextEditor::MainWindow *m_mainWindow = nullptr;

    QComboBox *m_modelsComboBox;
    QPushButton *m_newTabBtn;
    QPlainTextEdit *m_textAreaOutput;
    QPlainTextEdit *m_textAreaInput;
    QLabel *m_label_override_ollama_endpoint;
    QLineEdit *m_line_edit_override_ollama_endpoint;

    KateOllamaPlugin *m_plugin;
    OllamaSystem *m_ollamaSystem;
};
#endif // MAINTAB_H
