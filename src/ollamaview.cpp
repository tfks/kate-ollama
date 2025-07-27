/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "ollamaview.h"
#include "messages.h"
#include "plugin.h"

#include "ollamadata.h"

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

using namespace Qt::Literals::StringLiterals;

KateOllamaView::KateOllamaView(KateOllamaPlugin *plugin, KTextEditor::MainWindow *mainwindow)
    : KXMLGUIClient()
    , m_plugin(plugin)
    , m_mainWindow(mainwindow)
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
    connect(a, &QAction::triggered, this, &KateOllamaView::onSinglePrompt);

    QAction *a2 = ac->addAction(QStringLiteral("kateollama-full-prompt"));
    a2->setText(i18n("Run Ollama Full Text"));
    a2->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a2, QKeySequence((Qt::CTRL | Qt::SHIFT | Qt::Key_Semicolon)));
    connect(a2, &QAction::triggered, this, &KateOllamaView::onFullPrompt);

    QAction *a3 = ac->addAction(QStringLiteral("kateollama-command"));
    a3->setText(i18n("Add Ollama Command"));
    KActionCollection::setDefaultShortcut(a3, QKeySequence((Qt::CTRL | Qt::Key_Slash)));
    connect(a3, &QAction::triggered, this, &KateOllamaView::printCommand);

    m_mainWindow->guiFactory()->addClient(this);

    auto toolview =
        m_mainWindow->createToolView(plugin, "ollamatoolwindow", KTextEditor::MainWindow::Bottom, QIcon::fromTheme("applications-all"), i18n("Ollama"));

    m_toolview.reset(toolview);

    // Create custom widget here.

    // m_previewer = new QTextBrowser(m_toolview.get());
    //  Add the widget to our toolview
    // m_toolview->layout()->addWidget(m_previewer);
}

KateOllamaView::~KateOllamaView()
{
    m_mainWindow->guiFactory()->removeClient(this);
}

void KateOllamaView::printCommand()
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

void KateOllamaView::ollamaRequest(QString prompt)
{
    Messages::showStatusMessage(QStringLiteral("Info: Setting up request..."), KTextEditor::Message::Information, m_mainWindow);
    KTextEditor::View *view = m_mainWindow->activeView();
    KTextEditor::Document *document = view->document();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(m_plugin->getOllamaUrl() + "/api/generate"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json_data;
    OllamaData data;
    QVector<QString> images;

    data.setModel(m_plugin->getModel());
    data.setPrompt(prompt);
    data.setSuffix("");

    for (int i = 0; i < images.size(); ++i) {
        data.addImage(images[i]);
    }

    // data.setFormat("");
    // data.setOptions("");
    data.setSystem(m_plugin->getSystemPrompt());
    // data.setContext("");
    // data.setStream("");

    /*
     *
     */

    json_data = data.toJson();

    QJsonDocument doc(json_data);

    QNetworkReply *reply = manager->post(request, doc.toJson());

    connect(reply, &QNetworkReply::metaDataChanged, this, [=, this]() {
        KTextEditor::Cursor cursor = view->cursorPosition();
        document->insertText(cursor, "\n");
        Messages::showStatusMessage(QStringLiteral("Info: Request started..."), KTextEditor::Message::Information, m_mainWindow);
    });

    connect(reply, &QNetworkReply::readyRead, this, [this, reply, view, document]() {
        QString responseChunk = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseChunk.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();

        if (jsonObj.contains("response")) {
            Messages::showStatusMessage(QStringLiteral("Info: Reply received..."), KTextEditor::Message::Information, m_mainWindow);
            QString responseText = jsonObj["response"].toString();

            KTextEditor::Cursor cursor = view->cursorPosition();
            document->insertText(cursor, responseText);
        }
    });

    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        if (reply->error() != QNetworkReply::NoError) {
            Messages::showStatusMessage(QStringLiteral("Error encountered: ").arg(reply->errorString()), KTextEditor::Message::Information, m_mainWindow);
            qDebug() << "Error:" << reply->errorString();
            qDebug() << "Model:" << m_plugin->getModel();
            qDebug() << "System prompt:" << m_plugin->getSystemPrompt();
        }
        reply->deleteLater();

        KTextEditor::Cursor cursor = view->cursorPosition();
        document->insertText(cursor, "\n");
    });
}

QString KateOllamaView::getPrompt()
{
    Messages::showStatusMessage(QStringLiteral("Info: Getting prompt..."), KTextEditor::Message::Information, m_mainWindow);
    KTextEditor::View *view = m_mainWindow->activeView();
    KTextEditor::Document *document = view->document();
    QString text = document->text();

    QRegularExpression re("// AI:(.*)");
    QRegularExpressionMatchIterator matchIterator = re.globalMatch(text);

    QString lastMatch;

    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        lastMatch = match.captured(1).trimmed();
    }
    qDebug() << "Ollama prompt:" << lastMatch;

    return lastMatch;
}

void KateOllamaView::onSinglePrompt()
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

void KateOllamaView::onFullPrompt()
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
