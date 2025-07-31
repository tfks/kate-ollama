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
#include <qevent.h>

#include "src/ollama/ollamaresponse.h"
#include "src/ollama/ollamasystem.h"
#include "src/plugin.h"
#include "src/ui/controls//qollamaplaintextedit.h"
#include "src/ui/widgets/toolwidget.h"

class MainTab : public QWidget, public KXMLGUIClient
{
    Q_OBJECT

public:
    MainTab(KateOllamaPlugin *plugin, KTextEditor::MainWindow *mainWindow, OllamaSystem *ollamaSystem, OllamaToolWidget *parent = nullptr);
    ~MainTab();

public slots:
    void handle_signalModelsListLoaded(const QList<QJsonValue> &modelsList);
    void handle_signalOnSinglePrompt();
    void handle_signalOnFullPrompt();

    void handle_signalOllamaRequestMetaDataChanged(OllamaResponse ollamaResponse);
    void handle_signalOllamaRequestGotResponse(OllamaResponse ollamaResponse);
    void handle_signalOllamaRequestFinished(OllamaResponse ollamaResponse);

    void handle_signal_textAreaInputEnterKeyWasPressed(QKeyEvent *event);

private:
    void loadModels();
    QString getPrompt();
    void ollamaRequest(QString prompt);

    KTextEditor::MainWindow *m_mainWindow = nullptr;

    QComboBox *m_modelsComboBox;
    QPushButton *m_newTabBtn;
    QOllamaPlainTextEdit *m_textAreaInput;
    QOllamaPlainTextEdit *m_textAreaOutput;
    QLabel *m_label_override_ollama_endpoint;
    QLineEdit *m_line_edit_override_ollama_endpoint;

    KateOllamaPlugin *m_plugin;
    OllamaSystem *m_ollamaSystem;
};
#endif // MAINTAB_H
