/*
    SPDX-FileCopyrightText: 2025 Daniele Mte90 Scasciafratte <mte90net@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEOLLAMAPLUGIN_H
#define KATEOLLAMAPLUGIN_H

// KF headers
#include <KTextEditor/Plugin>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>
#include <KXMLGUIClient>

class KateOllamaPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit KateOllamaPlugin(QObject *parent, const QVariantList & = QVariantList());

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};
class KateOllamaView : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    explicit KateOllamaView(KateOllamaPlugin *plugin, KTextEditor::MainWindow *mainwindow);
    ~KateOllamaView();

private slots:
    void onActionTriggered();
    void printCommand();

private:
    KTextEditor::MainWindow *m_mainWindow = nullptr;
};
#endif // KATEOLLAMAPLUGIN_H
