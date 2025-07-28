/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include <KLocalizedString>
#include <KPluginFactory>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <QHBoxLayout>
#include <QIcon>
#include <QObject>
#include <QVBoxLayout>

#include "maintab.h"
#include "toolwidget.h"

OllamaToolWidget::OllamaToolWidget(KTextEditor::MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
{
    new QVBoxLayout(this);
    layout()->setContentsMargins({});
    layout()->addWidget(&m_tabWidget);

    m_tabWidget.addTab(new MainTab(mainWindow, this), QStringLiteral("Ollama"));
    m_tabWidget.setTabsClosable(true);
    connect(&m_tabWidget, &QTabWidget::tabCloseRequested, this, [this](int idx) {
        if (auto w = m_tabWidget.widget(idx)) {
            w->deleteLater();
        }
        m_tabWidget.removeTab(idx);
    });
}

OllamaToolWidget::~OllamaToolWidget()
{
}

void OllamaToolWidget::newTab()
{
}
