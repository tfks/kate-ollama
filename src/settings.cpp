/*
 *  SPDX-FileCopyrightText: 2025 Daniele Mte90 Scasciafratte <mte90net@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "settings.h"
#include "plugin.h"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KTextEditor/ConfigPage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>

KateOllamaConfigPage::KateOllamaConfigPage(QWidget *parent, KateOllamaPlugin *plugin)
    : KTextEditor::ConfigPage(parent)
    , m_plugin(plugin)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Available Models
    {
        auto *hl = new QHBoxLayout;

        auto label = new QLabel(i18n("Available Models"));
        hl->addWidget(label);

        m_modelsComboBox = new QComboBox(this);
        hl->addWidget(m_modelsComboBox);

        layout->addLayout(hl);
    }

    // System Prompt
    {
        auto *hl = new QHBoxLayout;

        auto label = new QLabel(i18n("System Prompt"));
        hl->addWidget(label);

        m_systemPromptEdit = new QLineEdit(this);
        hl->addWidget(m_systemPromptEdit);

        layout->addLayout(hl);
    }

    layout->addStretch();

    setLayout(layout);

    fetchModelList();
}

void KateOllamaConfigPage::fetchModelList()
{
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
                            m_modelsComboBox->addItem(modelValue.toString());
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

void KateOllamaConfigPage::apply()
{
    saveSettings();
}

void KateOllamaConfigPage::defaults()
{
}

void KateOllamaConfigPage::reset()
{
    // Reset the UI values to last known settings
    m_modelsComboBox->setCurrentText(m_plugin->model);
    m_systemPromptEdit->setText(m_plugin->systemPrompt);
}

void KateOllamaConfigPage::saveSettings()
{
    // Save settings to disk
    KConfigGroup group(KSharedConfig::openConfig(), "KateOllama");
    group.writeEntry("Model", m_modelsComboBox->currentText());
    group.writeEntry("SystemPrompt", m_systemPromptEdit->text());
    group.sync();

    // Update the cached variables in Plugin
    m_plugin->model = m_modelsComboBox->currentText();
    m_plugin->systemPrompt = m_systemPromptEdit->text();
}
