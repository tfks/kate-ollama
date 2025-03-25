/*
    SPDX-FileCopyrightText: 2025 Daniele Mte90 Scasciafratte <mte90net@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateollamaview.h"

#include "kateollamaplugin.h"

// KF headers
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/MainWindow>

#include <KLocalizedString>


KateOllamaView::KateOllamaView(KateOllamaPlugin* plugin, KTextEditor::MainWindow* mainwindow)
    : QObject(mainwindow)
{
    Q_UNUSED(plugin);
}

KateOllamaView::~KateOllamaView()
{
}

#include "moc_kateollamaview.cpp"
