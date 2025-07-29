/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef OLLAMAGLOBALS_H
#define OLLAMAGLOBALS_H

#include <QIcon>
#include <QObject>
#include <QString>

class OllamaGlobals : public QObject
{
    Q_OBJECT

public:
    static QString PluginName;
    static QString IconName;

    static QString HelpText;

    static QString LabelOllamaEndpointOverride;
};

#endif // OLLAMAGLOBALS_H
