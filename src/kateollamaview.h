/*
    SPDX-FileCopyrightText: 2025 Daniele Mte90 Scasciafratte <mte90net@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEOLLAMAVIEW_H
#define KATEOLLAMAVIEW_H

// Qt headers
#include <QObject>

namespace KTextEditor {
class MainWindow;
}

class KateOllamaPlugin;

class KateOllamaView: public QObject
{
    Q_OBJECT

public:
    KateOllamaView(KateOllamaPlugin* plugin, KTextEditor::MainWindow *view);
    ~KateOllamaView() override;
};

#endif // KATEOLLAMAVIEW_H
