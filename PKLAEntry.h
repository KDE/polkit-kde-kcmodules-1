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

class QDBusArgument;

class PKLAEntry {
    public:
    QString title;
    QString identity;
    QString action;
    QString resultAny;
    QString resultInactive;
    QString resultActive;

    int filePriority;
    int fileOrder;
};
Q_DECLARE_METATYPE(PKLAEntry)

typedef QList<PKLAEntry> PKLAEntryList;

QDBusArgument& operator<<(QDBusArgument& argument, const PKLAEntry& entry);
const QDBusArgument& operator>>(const QDBusArgument& argument, PKLAEntry& entry);

#endif // PKLAENTRY_H
