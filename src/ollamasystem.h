/*
 *  SPDX-FileCopyrightText: 2025 tfks <development@worloflinux.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef OLLAMASYSTEM_H
#define OLLAMASYSTEM_H

#include <QJsonArray>

#include "src/ollamadata.h"

class OllamaSystem : QObject
{
public:
    OllamaSystem(QObject *parent);
    ~OllamaSystem();

    QList<QJsonValue> fetchModels();
    void ollamaRequest(OllamaData data);

private:
    QObject *parent = nullptr;
    QList<QJsonValue> m_modelsList;
    QStringList m_errors;
    QStringList m_messages;
};

#endif // OLLAMASYSTEM_H
