/* This file is part of the KDE project

   Copyright (C) 2008 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "PKLAEntry.h"
#include <QtDBus/qdbusargument.h>

QDBusArgument &operator<<(QDBusArgument &argument, const PKLAEntry &entry)
{
    argument.beginStructure();
    argument << entry.title << entry.identity << entry.action << entry.resultAny << entry.resultInactive << entry.resultActive << entry.filePath
             << entry.filePriority << entry.fileOrder;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, PKLAEntry &entry)
{
    argument.beginStructure();
    argument >> entry.title >> entry.identity >> entry.action >> entry.resultAny >> entry.resultInactive >> entry.resultActive >> entry.filePath
             >> entry.filePriority >> entry.fileOrder;
    argument.endStructure();
    return argument;
}

PolkitQt1::ActionDescription::ImplicitAuthorization PKLAEntry::implFromText(const QString &text)
{
    if (text == QLatin1String("yes")) {
        return PolkitQt1::ActionDescription::Authorized;
    } else if (text == QLatin1String("no")) {
        return PolkitQt1::ActionDescription::NotAuthorized;
    } else if (text == QLatin1String("auth_admin")) {
        return PolkitQt1::ActionDescription::AdministratorAuthenticationRequired;
    } else if (text == QLatin1String("auth_admin_keep")) {
        return PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained;
    } else if (text == QLatin1String("auth_self")) {
        return PolkitQt1::ActionDescription::AuthenticationRequired;
    } else if (text == QLatin1String("auth_self_keep")) {
        return PolkitQt1::ActionDescription::AuthenticationRequiredRetained;
    } else {
        return PolkitQt1::ActionDescription::Unknown;
    }
}

QString PKLAEntry::textFromImpl(PolkitQt1::ActionDescription::ImplicitAuthorization implicit)
{
    switch (implicit) {
    case PolkitQt1::ActionDescription::Authorized:
        return QLatin1String("yes");
    case PolkitQt1::ActionDescription::NotAuthorized:
        return QLatin1String("no");
    case PolkitQt1::ActionDescription::AdministratorAuthenticationRequired:
        return QLatin1String("auth_admin");
    case PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained:
        return QLatin1String("auth_admin_keep");
    case PolkitQt1::ActionDescription::AuthenticationRequired:
        return QLatin1String("auth_self");
    case PolkitQt1::ActionDescription::AuthenticationRequiredRetained:
        return QLatin1String("auth_self_keep");
    default:
        return QString();
    }
}

