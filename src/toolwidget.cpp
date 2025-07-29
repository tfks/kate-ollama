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
#include "ollamaglobals.h"
#include "toolwidget.h"

OllamaToolWidget::OllamaToolWidget(KateOllamaPlugin *plugin, KTextEditor::MainWindow *mainWindow, OllamaSystem *ollamaSystem, QWidget *parent)
    : QWidget(parent)
    , m_plugin(plugin)
    , m_mainWindow(mainWindow)
    , m_ollamaSystem(ollamaSystem)
{
    new QVBoxLayout(this);
    layout()->setContentsMargins({});
    layout()->addWidget(&m_tabWidget);

    m_tabWidget.addTab(new MainTab(m_plugin, mainWindow, m_ollamaSystem, this), OllamaGlobals::PluginName);
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
    int index = m_tabWidget.count() + 1;

    QString tabName = QString(OllamaGlobals::PluginName).append(" (").append(QString::number(index)).append(")");

    int i = m_tabWidget.addTab(new MainTab(m_plugin, m_mainWindow, m_ollamaSystem, this), tabName);

    m_tabWidget.setCurrentIndex(i);
}
