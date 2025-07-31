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
    , m_mainWindow(mainWindow)
    , m_plugin(plugin)
    , m_ollamaSystem(ollamaSystem)
{
    auto l = new QVBoxLayout(this);

    m_modelsComboBox = new QComboBox(this);
    m_newTabBtn = new QPushButton(QIcon::fromTheme(QStringLiteral("tab-new")), QString());
    m_newTabBtn->setToolTip(i18n("Add new tab"));

    m_textAreaInput = new QOllamaPlainTextEdit(this);
    m_textAreaInput->setPlaceholderText(ki18n(OllamaGlobals::HelpText.toUtf8().data()).toString());

    m_textAreaOutput = new QOllamaPlainTextEdit(this);

    m_label_override_ollama_endpoint = new QLabel(ki18n(OllamaGlobals::LabelOllamaEndpointOverride.toUtf8().data()).toString(), this);
    m_line_edit_override_ollama_endpoint = new QLineEdit(m_plugin->getOllamaUrl(), this);

    // I want to add a toggle button to let ollama write code in the editor.

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

    hl->addWidget(m_modelsComboBox);
    hl->addWidget(m_newTabBtn);

    l->addLayout(hl);

    auto h2 = new QHBoxLayout();

    h2->addWidget(m_label_override_ollama_endpoint);
    h2->addWidget(m_line_edit_override_ollama_endpoint);
    h2->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    m_modelsComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_newTabBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_label_override_ollama_endpoint->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_line_edit_override_ollama_endpoint->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_line_edit_override_ollama_endpoint->setFixedWidth(200);

    l->addWidget(m_textAreaOutput, 1);
    l->addWidget(m_textAreaInput, 1);

    l->addLayout(h2);

    l->addStretch();

    connect(m_newTabBtn, &QAbstractButton::clicked, parent, &OllamaToolWidget::newTab);

    connect(a, &QAction::triggered, this, &MainTab::handle_signalOnSinglePrompt); // Need to get the last line in m_textAreaInput

    connect(a2, &QAction::triggered, this, &MainTab::handle_signalOnFullPrompt);

    connect(a3, &QAction::triggered, this, &MainTab::handle_signalOnFullPrompt);

    connect(m_ollamaSystem, &OllamaSystem::signal_modelsListLoaded, this, &MainTab::handle_signalModelsListLoaded);

    connect(m_ollamaSystem, &OllamaSystem::signal_ollamaRequestMetaDataChanged, this, &MainTab::handle_signalOllamaRequestMetaDataChanged);

    connect(m_ollamaSystem, &OllamaSystem::signal_ollamaRequestGotResponse, this, &MainTab::handle_signalOllamaRequestGotResponse);

    connect(m_ollamaSystem, &OllamaSystem::signal_ollamaRequestFinished, this, &MainTab::handle_signalOllamaRequestFinished);

    connect(m_textAreaInput,
            &QOllamaPlainTextEdit::signal_enterKeyWasPressed,
            this,
            &MainTab::handle_signal_textAreaInputEnterKeyWasPressed); // Also need to capture control enter for the full prompt.

    connect(m_textAreaOutput, &QPlainTextEdit::textChanged, m_textAreaOutput, &QOllamaPlainTextEdit::onTextChanged);

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
            m_modelsComboBox->addItem(modelObj["name"].toString());
        }

        if (modelObj["name"].toString() == m_plugin->getModel()) {
            modelSelected = m_modelsComboBox->count();
        }
    }

    if (modelSelected != -1) {
        m_modelsComboBox->setCurrentIndex(modelSelected - 1);
    }
}

void MainTab::handle_signalOnSinglePrompt()
{
    KTextEditor::View *view = m_mainWindow->activeView();
    if (view) {
        QString prompt = getPrompt();
        if (!prompt.isEmpty()) {
            Messages::showStatusMessage(QStringLiteral("Info: Single prompt.."), KTextEditor::Message::Information, m_mainWindow);
            ollamaRequest(prompt);
        } else {
            Messages::showStatusMessage(QStringLiteral("Info: No single prompt..."), KTextEditor::Message::Information, m_mainWindow);
        }
    } else {
        Messages::showStatusMessage(QStringLiteral("Info: Single prompt, no view..."), KTextEditor::Message::Information, m_mainWindow);
    }
}

void MainTab::handle_signalOnFullPrompt()
{
    QString prompt = m_textAreaInput->document()->toPlainText();
    ollamaRequest(prompt);
}

void MainTab::handle_signalOllamaRequestMetaDataChanged(OllamaResponse ollamaResponse)
{
    if (ollamaResponse.getReceiver() == "widget" || ollamaResponse.getReceiver() == "") {
        QTextCursor cursor = m_textAreaInput->textCursor();
        cursor.insertText("\n");

        Messages::showStatusMessage(QStringLiteral("Info: Request started..."), KTextEditor::Message::Information, m_mainWindow);
    }
}

void MainTab::handle_signalOllamaRequestGotResponse(OllamaResponse ollamaResponse)
{
    if (ollamaResponse.getReceiver() != "widget" && ollamaResponse.getReceiver() != "")
        return;

    QTextCursor cursor = m_textAreaOutput->textCursor();
    cursor.insertText(ollamaResponse.getResponseText());

    Messages::showStatusMessage(QStringLiteral("Info: Reply received..."), KTextEditor::Message::Information, m_mainWindow);
}

void MainTab::handle_signalOllamaRequestFinished(OllamaResponse ollamaResponse)
{
    if (ollamaResponse.getErrorMessage() != QString("")) {
        Messages::showStatusMessage(QStringLiteral("Error encountered: ").arg(ollamaResponse.getErrorMessage()),
                                    KTextEditor::Message::Information,
                                    m_mainWindow);
        qDebug() << "Error:" << ollamaResponse.getErrorMessage();
        qDebug() << "Model:" << m_plugin->getModel();
        qDebug() << "System prompt:" << m_plugin->getSystemPrompt();
    }

    if (ollamaResponse.getReceiver() == "widget" || ollamaResponse.getReceiver() == "") {
        QTextCursor cursor = m_textAreaOutput->textCursor();
        cursor.insertText("\n\n");
    }
}

void MainTab::handle_signal_textAreaInputEnterKeyWasPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        // Handle the Enter or Return key press here
        qDebug() << "Enter/Return key pressed in CustomWidget";

        QString prompt = m_textAreaInput->toPlainText();
        ollamaRequest(prompt);
    }
}

void MainTab::loadModels()
{
    OllamaData ollamaData;

    ollamaData.setOllamaUrl(m_plugin->getOllamaUrl());

    m_ollamaSystem->fetchModels(ollamaData);
}

QString MainTab::getPrompt()
{
    Messages::showStatusMessage(QStringLiteral("Info: Getting prompt..."), KTextEditor::Message::Information, m_mainWindow);

    QString text = m_textAreaInput->toPlainText();

    return m_ollamaSystem->getPromptFromText(text);
}

void MainTab::ollamaRequest(QString prompt)
{
    OllamaData data;

    QVector<QString> images;

    data.setSender("widget");
    data.setOllamaUrl(m_plugin->getOllamaUrl());
    data.setModel(m_plugin->getModel());
    data.setPrompt(prompt);
    data.setSuffix("");

    for (int i = 0; i < images.size(); ++i) {
        data.addImage(images[i]);
    }

    // data.setFormat("");
    // data.setOptions("");
    data.setSystemPrompt(m_plugin->getSystemPrompt());
    // data.setContext("");
    // data.setStream("");

    // we need to connect to the response as that is asynchronous.
    m_ollamaSystem->ollamaRequest(data);
}
