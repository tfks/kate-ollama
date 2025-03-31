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

#include <QComboBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>

KateOllamaConfigPage::KateOllamaConfigPage(QWidget *parent, KateOllamaPlugin *plugin)
    : KTextEditor::ConfigPage(parent)
    , m_plugin(plugin)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // URL
    {
        auto *hl = new QHBoxLayout;

        auto label = new QLabel(i18n("Ollama URL"));
        hl->addWidget(label);

        m_ollamaURLText = new QLineEdit(this);
        hl->addWidget(m_ollamaURLText);

        layout->addLayout(hl);
    }

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

        m_systemPromptEdit = new QTextEdit(this);
        m_systemPromptEdit->setGeometry(100, 100, 400, 300);
        hl->addWidget(m_systemPromptEdit);

        layout->addLayout(hl);
    }

    layout->addStretch();

    // Error/Info label
    {
        m_infoLabel = new QLabel(this);
        m_infoLabel->setVisible(false); // its hidden initially
        m_infoLabel->setWordWrap(true);
        layout->addWidget(m_infoLabel);
    }

    setLayout(layout);
    
    defaults();
    
    fetchModelList();
}

void KateOllamaConfigPage::fetchModelList()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            m_infoLabel->setVisible(false); // Hide the label on success

            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

            if (jsonDoc.isObject()) {
                QJsonObject jsonObj = jsonDoc.object();
                if (jsonObj.contains("models") && jsonObj["models"].isArray()) {
                    QJsonArray modelsArray = jsonObj["models"].toArray();
                    for (const QJsonValue &modelValue : modelsArray) {
                        QJsonObject modelObj = modelValue.toObject();
                        if (modelObj.contains("name")) {
                            m_modelsComboBox->addItem(modelObj["name"].toString());
                        }
                    }
                }
            }
        } else {
            qWarning() << "Error fetching model list:" << reply->errorString();
            // Show error in UI
            m_infoLabel->setText(i18n("Error fetching model list: %1", reply->errorString()));
        }
        reply->deleteLater();
    });

    QUrl url(m_ollamaURLText->text() + "/api/tags");
    QNetworkRequest request(url);
    manager->get(request);

    m_infoLabel->setText(i18n("Loading model list..."));
    m_infoLabel->setVisible(true);
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
    m_ollamaURLText->setText("http://localhost:11434");
    m_systemPromptEdit->setPlainText("You are a smart coder assistant, code comments are in the prompt language.");
}

void KateOllamaConfigPage::reset()
{
    // Reset the UI values to last known settings
    m_modelsComboBox->setCurrentText(m_plugin->model);
    m_systemPromptEdit->setPlainText(m_plugin->systemPrompt);
    m_ollamaURLText->setText(m_plugin->ollamaURL);
}

void KateOllamaConfigPage::saveSettings()
{
    // Save settings to disk
    KConfigGroup group(KSharedConfig::openConfig(), "KateOllama");
    group.writeEntry("Model", m_modelsComboBox->currentText());
    group.writeEntry("URL", m_ollamaURLText->text());
    group.writeEntry("SystemPrompt", m_systemPromptEdit->toPlainText());
    group.sync();

    // Update the cached variables in Plugin
    m_plugin->model = m_modelsComboBox->currentText();
    m_plugin->systemPrompt = m_systemPromptEdit->toPlainText();
    m_plugin->ollamaURL = m_ollamaURLText->text();
}
