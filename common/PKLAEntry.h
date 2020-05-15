/* This file is part of the KDE project

   Copyright (C) 2008 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef PKLAENTRY_H
#define PKLAENTRY_H

#include <QMetaType>
#include <polkit-qt5-1/PolkitQt1/ActionDescription>

#include <kdemacros.h>

class QDBusArgument;

class KDE_EXPORT PKLAEntry {
    public:
    QString title;
    QString identity;
    QString action;
    QString resultAny;
    QString resultInactive;
    QString resultActive;
    QString filePath;

    int filePriority;
    int fileOrder;

    // Static utils for PKLA
    static PolkitQt1::ActionDescription::ImplicitAuthorization implFromText(const QString& text);
    static QString textFromImpl(PolkitQt1::ActionDescription::ImplicitAuthorization implicit);
};
Q_DECLARE_METATYPE(PKLAEntry)
Q_DECLARE_METATYPE(QList<PKLAEntry>);

typedef QList<PKLAEntry> PKLAEntryList;

KDE_EXPORT QDBusArgument& operator<<(QDBusArgument& argument, const PKLAEntry& entry);
KDE_EXPORT const QDBusArgument& operator>>(const QDBusArgument& argument, PKLAEntry& entry);

#endif // PKLAENTRY_H
