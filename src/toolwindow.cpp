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

#include "toolwindow.h"

OllamaToolWindow::OllamaToolWindow(KTextEditor::MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
{
}

OllamaToolWindow::~OllamaToolWindow()
{
}
