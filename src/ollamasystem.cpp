/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

// KF Headers
#include <KLocalizedString>

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringLiteral>

#include "messages.h"
#include "ollamadata.h"
#include "ollamasystem.h"

OllamaSystem::OllamaSystem(QObject *parent)
    : parent(parent)
{
}

OllamaSystem::~OllamaSystem()
{
}

QList<QJsonValue> OllamaSystem::fetchModels()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(parent);
    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

            if (jsonDoc.isObject()) {
                QJsonObject jsonObj = jsonDoc.object();
                if (jsonObj.contains("models") && jsonObj["models"].isArray()) {
                    QJsonArray modelsArray = jsonObj["models"].toArray();

                    for (const QJsonValue &value : modelsArray) {
                        m_modelsList.append(value);
                    }
                    std::sort(m_modelsList.begin(), m_modelsList.end(), [](const QJsonValue &a, const QJsonValue &b) {
                        return a.toObject()["name"].toString().toLower() < b.toObject()["name"].toString().toLower();
                    });
                }
            }
        } else {
            qWarning() << "Error fetching model list:" << reply->errorString();
            m_errors.append(i18n("Error fetching model list: %1", reply->errorString()));
        }
        reply->deleteLater();
    });

    return m_modelsList;
}

void OllamaSystem::ollamaRequest(OllamaData data)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(data.getOllamaUrl() + "/api/generate"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json_data;
    json_data = data.toJson();

    QJsonDocument doc(json_data);

    QNetworkReply *reply = manager->post(request, doc.toJson());

    connect(reply, &QNetworkReply::metaDataChanged, this, [=, this]() {
        // Need to emit a signal metaDataChanged with a message that the info request was started.
    });

    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        QString responseChunk = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseChunk.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();

        if (jsonObj.contains("response")) {
            QString responseText = jsonObj["response"].toString();

            // Need to emit a signal that the data was recieved with responseText as data.
        }
    });

    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error:" << reply->errorString();
            qDebug() << "Model:" << data.getModel();
            qDebug() << "System prompt:" << data.getSystemPrompt();

            // Need to emit a signal that something went wrong with the errorString as data.
        }
        reply->deleteLater();
    });
}
