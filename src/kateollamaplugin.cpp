/*
    SPDX-FileCopyrightText: 2025 Daniele Mte90 Scasciafratte <mte90net@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateollamaplugin.h"

#include "kateollamaview.h"

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
#include <KLocalizedString>

K_PLUGIN_CLASS_WITH_JSON(KateOllamaPlugin, "kateollama.json")


KateOllamaPlugin::KateOllamaPlugin(QObject* parent, const QVariantList& /*args*/)
    : KTextEditor::Plugin(parent)
{
    QAction *action = new QAction(tr("Run Ollama"), this);
    connect(action, &QAction::triggered, this, &KateOllamaPlugin::onActionTriggered);

    // Add the action to the plugin's action collection
    KActionCollection *actionCollection = new KActionCollection(this);
    actionCollection->addAction("run_ollama", action);

    // Add the action to the GUI client
    if (KXMLGUIClient *guiClient = dynamic_cast<KXMLGUIClient*>(parent)) {
        actionCollection->setDefaultShortcut(action, Qt::CTRL + Qt::Key_O);
        guiClient->actionCollection()->addAction("run_ollama", action);
    }
}

void KateOllamaPlugin::onActionTriggered()
{
    KTextEditor::View *view = qobject_cast<KTextEditor::View*>(parent());
    if (view) {
        KTextEditor::Document *document = view->document();
        QString text = document->text();

        QRegularExpression re("// AI:(.*)");
        QRegularExpressionMatchIterator matchIterator = re.globalMatch(text);
        QStringList prompts;

        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            QString prompt = match.captured(1).trimmed();
            prompts.append(prompt);
        }

        if (!prompts.isEmpty()) {
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
                QString response = reply->readAll();
                KTextEditor::Cursor cursor = view->cursorPosition();

                // Insert the AI response at the cursor position or replace the prompts with the response
                document->insertText(cursor, response);

                reply->deleteLater();
            });

            QNetworkRequest request(QUrl("http://localhost:8000/api/v1/ollama")); // Adjust the URL as needed
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

            QJsonObject json;
            json.insert("model", "llama3");
            json.insert("prompt", prompts.join("\n"));  // Join prompts if there are multiple
            QJsonDocument doc(json);

            manager->post(request, doc.toJson());
        }
    }
}

QObject* KateOllamaPlugin::createView(KTextEditor::MainWindow* mainwindow)
{
    return new KateOllamaView(this, mainwindow);
}


// needed for K_PLUGIN_CLASS_WITH_JSON
#include <kateollamaplugin.moc>

#include "moc_kateollamaplugin.cpp"
