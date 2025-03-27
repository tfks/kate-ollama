/*
    SPDX-FileCopyrightText: 2025 Daniele Mte90 Scasciafratte <mte90net@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plugin.h"

// KF headers
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/Plugin>

#include <QAction>
#include <KActionCollection>
#include <KXMLGUIClient>
#include <KLocalizedString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QAction>
#include <QRegularExpression>
#include <KPluginFactory>
#include <KXMLGUIFactory>
#include <KLocalizedString>
#include <QDebug>
#include <QMenu>

using namespace Qt::Literals::StringLiterals;

K_PLUGIN_FACTORY_WITH_JSON(KateOllamaFactory, "kateollama.json", registerPlugin<KateOllamaPlugin>();)


KateOllamaPlugin::KateOllamaPlugin(QObject* parent, const QVariantList&)
    : KTextEditor::Plugin(parent)
{
}

KateOllamaView::KateOllamaView(KateOllamaPlugin *, KTextEditor::MainWindow *mainwindow)
    : KXMLGUIClient()
    , m_mainWindow(mainwindow)
{
    KXMLGUIClient::setComponentName(u"kateollama"_s, i18n("Kate-Ollama"));
    
    auto ac = actionCollection();
    QAction *a = ac->addAction(QStringLiteral("kateollama"));
    a->setText(i18n("Run Ollama"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a, QKeySequence((Qt::CTRL | Qt::Key_Semicolon)));
    connect(a, &QAction::triggered, this, &KateOllamaView::onActionTriggered);

    QAction *a2 = ac->addAction(QStringLiteral("kateollama-command"));
    a2->setText(i18n("Add Ollama Command"));
    KActionCollection::setDefaultShortcut(a2, QKeySequence((Qt::CTRL | Qt::Key_Slash)));
    connect(a2, &QAction::triggered, this, &KateOllamaView::printCommand);

    m_mainWindow->guiFactory()->addClient(this);
}

void KateOllamaView::printCommand()
{
    KTextEditor::View *view = m_mainWindow->activeView();;
    if (view) {
        KTextEditor::Document *document = view->document();
        QString text = document->text();
        KTextEditor::Cursor cursor = view->cursorPosition();
        document->insertText(cursor, "// AI: ");
    }
}

void KateOllamaView::onActionTriggered()
{
    KTextEditor::View *view = m_mainWindow->activeView();;
    if (view) {
        KTextEditor::Document *document = view->document();
        QString text = document->text();

        QRegularExpression re("// AI:(.*)");
        QRegularExpressionMatchIterator matchIterator = re.globalMatch(text);
        QStringList prompts;
        QString lastPrompt;

        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            QString prompt = match.captured(1).trimmed();
            prompts.append(prompt);
            lastPrompt = prompt;
        }

        if (!lastPrompt.isEmpty()) { // If you use prompts use also the past prompts
            qDebug() << "Ollama prompt:" << lastPrompt;
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);

            QNetworkRequest request(QUrl("http://localhost:11434/api/generate"));
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

            QJsonObject json;
            json.insert("model", "llama3.2:latest");
            json.insert("prompt", lastPrompt); //prompts.join("\n")
            QJsonDocument doc(json);

            QNetworkReply *reply = manager->post(request, doc.toJson());

            connect(reply, &QNetworkReply::metaDataChanged, this, [=]() {
                    KTextEditor::Cursor cursor = view->cursorPosition();
                    document->insertText(cursor, "\n");
            });
            
            connect(reply, &QNetworkReply::readyRead, this, [=]() {
                QString responseChunk = reply->readAll();
                QJsonDocument jsonDoc = QJsonDocument::fromJson(responseChunk.toUtf8());
                QJsonObject jsonObj = jsonDoc.object();

                if (jsonObj.contains("response")) {
                    QString responseText = jsonObj["response"].toString();
                    
                    KTextEditor::Cursor cursor = view->cursorPosition();
                    document->insertText(cursor, responseText);
                }
            });

            connect(reply, &QNetworkReply::finished, this, [=]() {
                if (reply->error() != QNetworkReply::NoError) {
                    qDebug() << "Error:" << reply->errorString();
                }
                reply->deleteLater();

                KTextEditor::Cursor cursor = view->cursorPosition();
                document->insertText(cursor, "\n");
            });
        }
    }
}

KateOllamaView::~KateOllamaView()
{
    m_mainWindow->guiFactory()->removeClient(this);
}

QObject* KateOllamaPlugin::createView(KTextEditor::MainWindow* mainwindow)
{
    return new KateOllamaView(this, mainwindow);
}

#include <plugin.moc>
