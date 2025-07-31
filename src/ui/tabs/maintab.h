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
#include <QSplitter>
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
    void handle_signalOutputInEditorClicked();

private:
    void loadModels();
    QString getPrompt();
    void ollamaRequest(QString prompt);

    bool outputInEditor_;

    KTextEditor::MainWindow *mainWindow_ = nullptr;

    QComboBox *modelsComboBox_;
    QPushButton *outputInEditorPushButton_;
    QPushButton *newTabBtn_;
    QOllamaPlainTextEdit *textAreaInput_;
    QOllamaPlainTextEdit *textAreaOutput_;
    QSplitter *splitter_;
    QLabel *label_override_ollama_endpoint_;
    QLineEdit *line_edit_override_ollama_endpoint_;

    KateOllamaPlugin *plugin_;
    OllamaSystem *ollamaSystem_;
};
#endif // MAINTAB_H
