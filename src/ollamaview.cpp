/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "ollamaview.h"
#include "messages.h"
#include "plugin.h"

#include "ollamadata.h"
#include "src/ollamasystem.h"
#include "src/toolwidget.h"

// KF headers
#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KTextEditor/Application>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Message>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include <QAction>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QString>
#include <ktexteditor/message.h>
#include <qcontainerfwd.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "ollamaglobals.h"
#include "toolwidget.h"

using namespace Qt::Literals::StringLiterals;

KateOllamaView::KateOllamaView(KateOllamaPlugin *plugin, KTextEditor::MainWindow *mainwindow, OllamaSystem *ollamaSystem)
    : KXMLGUIClient()
    , m_plugin(plugin)
    , m_mainWindow(mainwindow)
    , m_ollamaSystem(ollamaSystem)
{
    KXMLGUIClient::setComponentName(u"kateollama"_s, i18n("Kate-Ollama"));
    KConfigGroup group(KSharedConfig::openConfig(), "KateOllama");

    m_plugin->setModel(group.readEntry("Model"));
    m_plugin->setSystemPrompt(group.readEntry("SystemPrompt"));
    m_plugin->setOllamaUrl(group.readEntry("URL"));

    auto ac = actionCollection();
    QAction *a = ac->addAction(QStringLiteral("kateollama"));
    a->setText(i18n("Run Ollama"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a, QKeySequence((Qt::CTRL | Qt::Key_Semicolon)));
    connect(a, &QAction::triggered, this, &KateOllamaView::handle_onSinglePrompt);

    QAction *a2 = ac->addAction(QStringLiteral("kateollama-full-prompt"));
    a2->setText(i18n("Run Ollama Full Text"));
    a2->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a2, QKeySequence((Qt::CTRL | Qt::SHIFT | Qt::Key_Semicolon)));
    connect(a2, &QAction::triggered, this, &KateOllamaView::handle_onFullPrompt);

    QAction *a3 = ac->addAction(QStringLiteral("kateollama-command"));
    a3->setText(i18n("Add Ollama Command"));
    KActionCollection::setDefaultShortcut(a3, QKeySequence((Qt::CTRL | Qt::Key_Slash)));
    connect(a3, &QAction::triggered, this, &KateOllamaView::handle_onPrintCommand);

    m_mainWindow->guiFactory()->addClient(this);

    auto toolview = m_mainWindow->createToolView(plugin,
                                                 "ollamatoolwidget",
                                                 KTextEditor::MainWindow::Bottom,
                                                 QIcon::fromTheme(OllamaGlobals::IconName),
                                                 OllamaGlobals::PluginName);

    m_toolWidget = new OllamaToolWidget(m_plugin, m_mainWindow, m_ollamaSystem, toolview);

    m_toolview.reset(toolview);

    connect(m_ollamaSystem, &OllamaSystem::signal_ollamaRequestMetaDataChanged, this, &KateOllamaView::handle_ollamaRequestMetaDataChanged);

    connect(m_ollamaSystem, &OllamaSystem::signal_ollamaRequestGotResponse, this, &KateOllamaView::handle_ollamaRequestGotResponse);

    connect(m_ollamaSystem, &OllamaSystem::signal_ollamaRequestFinished, this, &KateOllamaView::handle_ollamaRequestFinished);
}

KateOllamaView::~KateOllamaView()
{
    m_mainWindow->guiFactory()->removeClient(this);
}

void KateOllamaView::handle_onSinglePrompt()
{
    KTextEditor::View *view = m_mainWindow->activeView();
    if (view) {
        QString prompt = KateOllamaView::getPrompt();
        if (!prompt.isEmpty()) {
            Messages::showStatusMessage(QStringLiteral("Info: Single prompt.."), KTextEditor::Message::Information, m_mainWindow);
            KateOllamaView::ollamaRequest(prompt);
        } else {
            Messages::showStatusMessage(QStringLiteral("Info: No single prompt..."), KTextEditor::Message::Information, m_mainWindow);
        }
    } else {
        Messages::showStatusMessage(QStringLiteral("Info: Single prompt, no view..."), KTextEditor::Message::Information, m_mainWindow);
    }
}

void KateOllamaView::handle_onFullPrompt()
{
    KTextEditor::View *view = m_mainWindow->activeView();
    KTextEditor::Document *document = view->document();
    QString text = document->text();
    if (view) {
        QString prompt = KateOllamaView::getPrompt();
        if (!prompt.isEmpty()) {
            Messages::showStatusMessage(QStringLiteral("Info: Full prompt..."), KTextEditor::Message::Information, m_mainWindow);
            KateOllamaView::ollamaRequest(text + "\n" + prompt);
        } else {
            Messages::showStatusMessage(QStringLiteral("Info: No full prompt..."), KTextEditor::Message::Information, m_mainWindow);
        }
    } else {
        Messages::showStatusMessage(QStringLiteral("Info: Full prompt, no view..."), KTextEditor::Message::Information, m_mainWindow);
    }
}

void KateOllamaView::handle_onPrintCommand()
{
    Messages::showStatusMessage(QStringLiteral("Info: Printing command..."), KTextEditor::Message::Information, m_mainWindow);
    KTextEditor::View *view = m_mainWindow->activeView();
    ;
    if (view) {
        KTextEditor::Document *document = view->document();
        QString text = document->text();
        KTextEditor::Cursor cursor = view->cursorPosition();
        document->insertText(cursor, "// AI: ");
    }
}

void KateOllamaView::handle_ollamaRequestMetaDataChanged()
{
    KTextEditor::View *view = m_mainWindow->activeView();
    KTextEditor::Document *document = view->document();
    KTextEditor::Cursor cursor = view->cursorPosition();
    document->insertText(cursor, "\n");
    Messages::showStatusMessage(QStringLiteral("Info: Request started..."), KTextEditor::Message::Information, m_mainWindow);
}

void KateOllamaView::handle_ollamaRequestGotResponse(QString responseText)
{
    KTextEditor::View *view = m_mainWindow->activeView();
    KTextEditor::Document *document = view->document();
    KTextEditor::Cursor cursor = view->cursorPosition();
    document->insertText(cursor, responseText);
    Messages::showStatusMessage(QStringLiteral("Info: Reply received..."), KTextEditor::Message::Information, m_mainWindow);
}

void KateOllamaView::handle_ollamaRequestFinished(QString errorMessage)
{
    if (errorMessage != QString("")) {
        Messages::showStatusMessage(QStringLiteral("Error encountered: ").arg(errorMessage), KTextEditor::Message::Information, m_mainWindow);
        qDebug() << "Error:" << errorMessage;
        qDebug() << "Model:" << m_plugin->getModel();
        qDebug() << "System prompt:" << m_plugin->getSystemPrompt();
    }

    KTextEditor::View *view = m_mainWindow->activeView();
    KTextEditor::Document *document = view->document();
    KTextEditor::Cursor cursor = view->cursorPosition();
    document->insertText(cursor, "\n");
}

QString KateOllamaView::getPrompt()
{
    Messages::showStatusMessage(QStringLiteral("Info: Getting prompt..."), KTextEditor::Message::Information, m_mainWindow);
    KTextEditor::View *view = m_mainWindow->activeView();
    KTextEditor::Document *document = view->document();
    QString text = document->text();

    QString lastMatch = m_ollamaSystem->getPromptFromText(text);

    return lastMatch;
}

void KateOllamaView::ollamaRequest(QString prompt)
{
    Messages::showStatusMessage(QStringLiteral("Info: Setting up request..."), KTextEditor::Message::Information, m_mainWindow);

    QJsonObject json_data;
    OllamaData data;
    QVector<QString> images;

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

    json_data = data.toJson();

    QJsonDocument doc(json_data);

    m_ollamaSystem->ollamaRequest(data);
}
