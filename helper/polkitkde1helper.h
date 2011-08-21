/* This file is part of the KDE project

   Copyright (C) 2008 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef POLKITKDE1HELPER_H
#define POLKITKDE1HELPER_H

#include <QDir>
#include <QtCore/QObject>
#include <QtDBus/QDBusContext>
#include <QtDBus/qdbusargument.h>
#include "../PKLAEntry.h"

class PolkitKde1Helper : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.polkitkde1.helper")

    public:
        PolkitKde1Helper(QObject* parent = 0);
        virtual ~PolkitKde1Helper();

    public Q_SLOTS:
        void saveGlobalConfiguration(const QString &adminIdentities, int systemPriority, int policiesPriority);
        QVariantList retrievePolicies();
        void writePolicy(const QList<PKLAEntry> &policy);

    private:
        QVariantList entriesFromFile(int filePriority, const QString &fileContents);
        QString formatPKLAEntry(const PKLAEntry &entry);
        QVariantList reloadFileList();
        QFileInfoList oldNestedList;
};

#endif // POLKITKDE1HELPER_H
