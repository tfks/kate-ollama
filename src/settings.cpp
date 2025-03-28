/*
 *  SPDX-FileCopyrightText: 2025 Daniele Mte90 Scasciafratte <mte90net@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "settings.h"
#include "plugin.h"
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <KTextEditor/ConfigPage>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVBoxLayout>

KateOllamaConfigPage::KateOllamaConfigPage(QWidget *parent, KateOllamaPlugin *plugin)
    : KTextEditor::ConfigPage(parent)
    , m_plugin(plugin)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    comboBox = new QComboBox(this);
    layout->addWidget(comboBox);

    lineEdit = new QLineEdit(this);
    layout->addWidget(lineEdit);

    setLayout(layout);

    loadSettings();
    
    fetchModelList();
}

void KateOllamaConfigPage::fetchModelList() {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

            if (jsonDoc.isObject()) {
                QJsonObject jsonObj = jsonDoc.object();
                if (jsonObj.contains("models") && jsonObj["models"].isArray()) {
                    QJsonArray modelsArray = jsonObj["models"].toArray();
                    for (const QJsonValue &modelValue : modelsArray) {
                        if (modelValue.isString()) {
                            comboBox->addItem(modelValue.toString());
                        }
                    }
                }
            }
        } else {
            qWarning() << "Error fetching model list:" << reply->errorString();
        }
        reply->deleteLater();
    });

    QUrl url("http://localhost:11434/models");
    QNetworkRequest request(url);
    manager->get(request);
}

QString KateOllamaConfigPage::name() const
{
    return i18n("Ollama");
}

QString KateOllamaConfigPage::fullName() const
{
    return i18nc("Groupbox title", "Ollama Settings");
}

QIcon KateOllamaConfigPage::icon() const
{
    return QIcon::fromTheme(QLatin1String("project-open"), QIcon::fromTheme(QLatin1String("view-list-tree")));
}

void KateOllamaConfigPage::defaults()
{
    reset();
}

void KateOllamaConfigPage::saveSettings() {
    KConfigGroup group(KSharedConfig::openConfig(), "KateOllama");
    group.writeEntry("Model", comboBox->currentText());
    group.writeEntry("SystemPrompt", lineEdit->text());
    group.sync();
}

void KateOllamaConfigPage::loadSettings() {
    KConfigGroup group(KSharedConfig::openConfig(), "KateOllama");
    comboBox->setCurrentText(group.readEntry("Model", "llama3.2:latest"));
    lineEdit->setText(group.readEntry("SystemPrompt", ""));
}
