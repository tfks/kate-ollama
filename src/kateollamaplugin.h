/*
    SPDX-FileCopyrightText: 2025 Daniele Mte90 Scasciafratte <mte90net@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEOLLAMAPLUGIN_H
#define KATEOLLAMAPLUGIN_H

// KF headers
#include <KTextEditor/Plugin>

class KateOllamaPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    /**
     * Default constructor, with arguments as expected by KPluginFactory
     */
    KateOllamaPlugin(QObject* parent, const QVariantList& args);

    public: // KTextEditor::Plugin API
        QObject* createView(KTextEditor::MainWindow* mainWindow) override;

private slots:
    void onActionTriggered();
};
#endif // KATEOLLAMAPLUGIN_H
