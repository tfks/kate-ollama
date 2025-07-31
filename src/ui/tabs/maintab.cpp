/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
// KF Headers
#include <KActionCollection>
#include <KLocalizedString>
#include <KXMLGUIClient>

#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonValue>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLocale>
#include <QObject>
#include <QPlainTextEdit>
#include <QSizePolicy>
#include <QSplitter>
#include <QVBoxLayout>
#include <qlineedit.h>
#include <qnamespace.h>

#include "src/ollama/ollamadata.h"
#include "src/ollama/ollamaglobals.h"
#include "src/ollama/ollamaresponse.h"
#include "src/ollama/ollamasystem.h"
#include "src/ui/controls/qollamaplaintextedit.h"
#include "src/ui/tabs/maintab.h"
#include "src/ui/utilities/messages.h"
#include "src/ui/widgets/toolwidget.h"

MainTab::MainTab(KateOllamaPlugin *plugin, KTextEditor::MainWindow *mainWindow, OllamaSystem *ollamaSystem, OllamaToolWidget *parent)
    : QWidget(parent)
    , mainWindow_(mainWindow)
    , plugin_(plugin)
    , ollamaSystem_(ollamaSystem)
{
    auto l = new QVBoxLayout(this);

    modelsComboBox_ = new QComboBox(this);
    outputInEditorPushButton_ = new QPushButton(QIcon::fromTheme(QStringLiteral("text-x-generic")), QString(i18n("Output in editor (Off)")));
    outputInEditorPushButton_->setToolTip(i18n("Output in editor"));
    newTabBtn_ = new QPushButton(QIcon::fromTheme(QStringLiteral("tab-new")), QString());
    newTabBtn_->setToolTip(i18n("Add new tab"));

    textAreaInput_ = new QOllamaPlainTextEdit(this);
    textAreaInput_->setPlaceholderText(ki18n(OllamaGlobals::HelpText.toUtf8().data()).toString());
    textAreaOutput_ = new QOllamaPlainTextEdit(this);

    splitter_ = new QSplitter(Qt::Vertical);

    splitter_->setMinimumSize(100, 100);
    textAreaInput_->setMinimumHeight(50);
    textAreaInput_->setMinimumHeight(50);

    label_override_ollama_endpoint_ = new QLabel(ki18n(OllamaGlobals::LabelOllamaEndpointOverride.toUtf8().data()).toString(), this);
    line_edit_override_ollama_endpoint_ = new QLineEdit(plugin_->getOllamaUrl(), this);

    label_override_ollama_endpoint_->setMaximumHeight(15);
    line_edit_override_ollama_endpoint_->setMaximumHeight(15);

    auto ac = actionCollection();

    QAction *a = ac->addAction(QStringLiteral("kateollama-standard-enter"));
    a->setText(i18n("ask-question"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a, QKeySequence(Qt::Key_Enter));

    QAction *a2 = ac->addAction(QStringLiteral("kateollama-newline"));
    a2->setText(i18n("New line"));
    a2->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a2, QKeySequence((Qt::CTRL | Qt::Key_Enter)));

    QAction *a3 = ac->addAction(QStringLiteral("kateollama-newline-alt"));
    a3->setText(i18n("New line"));
    a3->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a2, QKeySequence((Qt::SHIFT | Qt::Key_Enter)));

    auto hl = new QHBoxLayout();

    hl->addWidget(modelsComboBox_);
    hl->addWidget(newTabBtn_);

    l->addLayout(hl);

    auto h2 = new QHBoxLayout();

    QSizePolicy spTop(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spTop.setVerticalPolicy(QSizePolicy::Preferred);

    QSizePolicy spBottom(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spBottom.setVerticalPolicy(QSizePolicy::Fixed);

    textAreaInput_->setSizePolicy(spTop);

    label_override_ollama_endpoint_->setSizePolicy(spBottom);
    line_edit_override_ollama_endpoint_->setSizePolicy(spBottom);

    h2->addWidget(label_override_ollama_endpoint_);
    h2->addWidget(line_edit_override_ollama_endpoint_);
    h2->addWidget(outputInEditorPushButton_);
    h2->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    modelsComboBox_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    newTabBtn_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    label_override_ollama_endpoint_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    line_edit_override_ollama_endpoint_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    line_edit_override_ollama_endpoint_->setFixedWidth(200);

    // l->addWidget(textAreaOutput_, 1);
    // l->addWidget(textAreaInput_, 1);
    l->addWidget(splitter_);

    splitter_->addWidget(textAreaOutput_);
    splitter_->addWidget(textAreaInput_);

    l->addLayout(h2);

    l->addStretch();

    connect(newTabBtn_, &QAbstractButton::clicked, parent, &OllamaToolWidget::newTab);

    connect(a, &QAction::triggered, this, &MainTab::handle_signalOnSinglePrompt);

    connect(a2, &QAction::triggered, this, &MainTab::handle_signalOnFullPrompt);

    connect(a3, &QAction::triggered, this, &MainTab::handle_signalOnFullPrompt);

    connect(ollamaSystem_, &OllamaSystem::signal_modelsListLoaded, this, &MainTab::handle_signalModelsListLoaded);

    connect(ollamaSystem_, &OllamaSystem::signal_ollamaRequestMetaDataChanged, this, &MainTab::handle_signalOllamaRequestMetaDataChanged);

    connect(ollamaSystem_, &OllamaSystem::signal_ollamaRequestGotResponse, this, &MainTab::handle_signalOllamaRequestGotResponse);

    connect(ollamaSystem_, &OllamaSystem::signal_ollamaRequestFinished, this, &MainTab::handle_signalOllamaRequestFinished);

    connect(textAreaInput_,
            &QOllamaPlainTextEdit::signal_enterKeyWasPressed,
            this,
            &MainTab::handle_signal_textAreaInputEnterKeyWasPressed); // Also need to capture control enter for the full prompt.

    connect(textAreaOutput_, &QPlainTextEdit::textChanged, textAreaOutput_, &QOllamaPlainTextEdit::onTextChanged);

    connect(outputInEditorPushButton_, &QPushButton::clicked, this, &MainTab::handle_signalOutputInEditorClicked);

    loadModels();
}

MainTab::~MainTab()
{
}

void MainTab::handle_signalModelsListLoaded(const QList<QJsonValue> &modelsList)
{
    int modelSelected = -1;
    for (const QJsonValue &modelValue : modelsList) {
        QJsonObject modelObj = modelValue.toObject();
        if (modelObj.contains("name")) {
            modelsComboBox_->addItem(modelObj["name"].toString());
        }

        if (modelObj["name"].toString() == plugin_->getModel()) {
            modelSelected = modelsComboBox_->count();
        }
    }

    if (modelSelected != -1) {
        modelsComboBox_->setCurrentIndex(modelSelected - 1);
    }
}

void MainTab::handle_signalOnSinglePrompt()
{
    KTextEditor::View *view = mainWindow_->activeView();
    if (view) {
        QString prompt = getPrompt();
        if (!prompt.isEmpty()) {
            Messages::showStatusMessage(QStringLiteral("Info: Single prompt.."), KTextEditor::Message::Information, mainWindow_);
            ollamaRequest(prompt);
        } else {
            Messages::showStatusMessage(QStringLiteral("Info: No single prompt..."), KTextEditor::Message::Information, mainWindow_);
        }
    } else {
        Messages::showStatusMessage(QStringLiteral("Info: Single prompt, no view..."), KTextEditor::Message::Information, mainWindow_);
    }
}

void MainTab::handle_signalOnFullPrompt()
{
    QString prompt = textAreaInput_->document()->toPlainText();
    ollamaRequest(prompt);
}

void MainTab::handle_signalOllamaRequestMetaDataChanged(OllamaResponse ollamaResponse)
{
    if (ollamaResponse.getReceiver() == "widget" || ollamaResponse.getReceiver() == "" || outputInEditor_) {
        QTextCursor cursor = textAreaInput_->textCursor();
        cursor.insertText("\n");

        Messages::showStatusMessage(QStringLiteral("Info: Request started..."), KTextEditor::Message::Information, mainWindow_);
    }
}

void MainTab::handle_signalOllamaRequestGotResponse(OllamaResponse ollamaResponse)
{
    if (ollamaResponse.getReceiver() != "widget" && ollamaResponse.getReceiver() != "")
        return;

    QTextCursor cursor = textAreaOutput_->textCursor();
    cursor.insertText(ollamaResponse.getResponseText());

    Messages::showStatusMessage(QStringLiteral("Info: Reply received..."), KTextEditor::Message::Information, mainWindow_);
}

void MainTab::handle_signalOllamaRequestFinished(OllamaResponse ollamaResponse)
{
    if (ollamaResponse.getErrorMessage() != QString("")) {
        Messages::showStatusMessage(QStringLiteral("Error encountered: ").arg(ollamaResponse.getErrorMessage()),
                                    KTextEditor::Message::Information,
                                    mainWindow_);
        qDebug() << "Error:" << ollamaResponse.getErrorMessage();
        qDebug() << "Model:" << plugin_->getModel();
        qDebug() << "System prompt:" << plugin_->getSystemPrompt();
    }

    if (ollamaResponse.getReceiver() == "widget" || ollamaResponse.getReceiver() == "") {
        QTextCursor cursor = textAreaOutput_->textCursor();
        cursor.insertText("\n\n");
    }
}

void MainTab::handle_signal_textAreaInputEnterKeyWasPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        // Handle the Enter or Return key press here
        qDebug() << "Enter/Return key pressed in CustomWidget";

        QString prompt = textAreaInput_->toPlainText();
        ollamaRequest(prompt);
    }
}

void MainTab::handle_signalOutputInEditorClicked()
{
    if (outputInEditor_ == false) {
        outputInEditor_ = true;
        outputInEditorPushButton_->setText(i18n("Output in editor (On)"));
    } else {
        outputInEditor_ = false;
        outputInEditorPushButton_->setText(i18n("Output in editor (Off)"));
    }
}

void MainTab::loadModels()
{
    OllamaData ollamaData;

    ollamaData.setOllamaUrl(plugin_->getOllamaUrl());

    ollamaSystem_->fetchModels(ollamaData);
}

QString MainTab::getPrompt()
{
    Messages::showStatusMessage(QStringLiteral("Info: Getting prompt..."), KTextEditor::Message::Information, mainWindow_);

    QString text = textAreaInput_->toPlainText();

    return ollamaSystem_->getPromptFromText(text);
}

void MainTab::ollamaRequest(QString prompt)
{
    OllamaData data;

    QVector<QString> images;

    if (outputInEditor_) {
        data.setSender("editor");
    } else {
        data.setSender("widget");
    }

    data.setOllamaUrl(plugin_->getOllamaUrl());
    data.setModel(plugin_->getModel());
    data.setPrompt(prompt);
    data.setSuffix("");

    for (int i = 0; i < images.size(); ++i) {
        data.addImage(images[i]);
    }

    // data.setFormat("");
    // data.setOptions("");
    data.setSystemPrompt(plugin_->getSystemPrompt());
    // data.setContext("");
    // data.setStream("");

    // we need to connect to the response as that is asynchronous.
    ollamaSystem_->ollamaRequest(data);
}
