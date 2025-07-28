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
#include <QVBoxLayout>
#include <qnamespace.h>

#include "maintab.h"
#include "ollamadata.h"
#include "ollamasystem.h"
#include "toolwidget.h"

MainTab::MainTab(KTextEditor::MainWindow *mainWindow, OllamaToolWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
{
    m_ollamaSystem = new OllamaSystem(this);

    auto l = new QVBoxLayout(this);

    m_modelsComboBox = new QComboBox(this);
    m_textAreaOutput = new QPlainTextEdit(this);
    m_newTabBtn = new QPushButton(QIcon::fromTheme(QStringLiteral("tab-new")), QString());
    m_newTabBtn->setToolTip(i18n("Add new tab"));

    connect(m_newTabBtn, &QAbstractButton::clicked, parent, &OllamaToolWidget::newTab);

    m_textAreaInput = new QPlainTextEdit(this);
    m_textAreaInput->setPlaceholderText(i18n("Ask a question, press Enter to send.\n(Tip: use CTRL+Enter or SHIFT+Enter for adding a new line)"));

    auto ac = actionCollection();

    QAction *a = ac->addAction(QStringLiteral("kateollama-standard-enter"));
    a->setText(i18n("ask-question"));
    a->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a, QKeySequence(Qt::Key_Enter));
    // connect(a, &QAction::triggered, this, &MainTab::onSinglePrompt); // Need to get the last line in m_textAreaInput

    QAction *a2 = ac->addAction(QStringLiteral("kateollama-newline"));
    a2->setText(i18n("New line"));
    a2->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a2, QKeySequence((Qt::CTRL | Qt::Key_Enter)));
    connect(a2, &QAction::triggered, this, &MainTab::onFullPrompt);

    QAction *a3 = ac->addAction(QStringLiteral("kateollama-newline-alt"));
    a3->setText(i18n("New line"));
    a3->setIcon(QIcon::fromTheme(QStringLiteral("debug-run")));
    KActionCollection::setDefaultShortcut(a2, QKeySequence((Qt::SHIFT | Qt::Key_Enter)));
    connect(a3, &QAction::triggered, this, &MainTab::onFullPrompt);

    // ...

    auto hl = new QHBoxLayout();
    hl->addWidget(m_modelsComboBox);
    hl->addWidget(m_textAreaOutput);
    hl->addWidget(m_newTabBtn);
    l->addLayout(hl);

    auto h2 = new QHBoxLayout();
    h2->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hl->addWidget(m_textAreaInput);
    hl->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    l->addLayout(h2);

    l->addStretch();

    loadModels();
}

MainTab::~MainTab()
{
}

void MainTab::loadModels()
{
    QList<QJsonValue> models = m_ollamaSystem->fetchModels();

    int modelSelected = -1;
    for (const QJsonValue &modelValue : models) {
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

void MainTab::onFullPrompt()
{
    QString prompt = m_textAreaInput->document()->toPlainText();

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
    data.setSystemPrompt(m_plugin->getSystemPrompt());
    // data.setContext("");
    // data.setStream("");

    // we need to connect to the response as that is asynchronous.
    m_ollamaSystem->ollamaRequest(data);
}
