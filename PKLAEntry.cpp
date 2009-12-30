/* This file is part of the KDE project

   Copyright (C) 2008 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "PKLAEntry.h"
#include <QtDBus/qdbusargument.h>

QDBusArgument& operator<<(QDBusArgument& argument, const PKLAEntry& entry)
{
    argument.beginStructure();
    argument << entry.title << entry.identity << entry.action << entry.resultAny << entry.resultInactive << entry.resultActive
             << entry.filePriority << entry.fileOrder;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, PKLAEntry& entry)
{
    argument.beginStructure();
    argument >> entry.title >> entry.identity >> entry.action >> entry.resultAny >> entry.resultInactive >> entry.resultActive
             >> entry.filePriority >> entry.fileOrder;
    argument.endStructure();
    return argument;
}

