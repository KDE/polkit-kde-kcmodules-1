/* This file is part of the KDE project

   Copyright (C) 2008 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "polkitkde1helper.h"

#include "helperadaptor.h"

#include <QTimer>
#include <QSettings>

#include <QtDBus/QDBusConnection>

#include <PolkitQt1/Authority>
#include <QDir>

bool orderByPriorityLessThan(const PKLAEntry &e1, const PKLAEntry &e2)
{
    return e1.fileOrder < e2.fileOrder;
}

PolkitKde1Helper::PolkitKde1Helper(QObject* parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<PKLAEntry>();
    (void) new HelperAdaptor(this);
    // Register the DBus service
    if (!QDBusConnection::systemBus().registerService("org.kde.polkitkde1.helper")) {
        qDebug() << QDBusConnection::systemBus().lastError().message();;
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }

    if (!QDBusConnection::systemBus().registerObject("/Helper", this)) {
        qDebug() << "unable to register service interface to dbus";
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }
}

PolkitKde1Helper::~PolkitKde1Helper()
{

}

void PolkitKde1Helper::saveGlobalConfiguration(const QString& adminIdentities, int systemPriority, int policiesPriority)
{
    qDebug() << "Request to save the global configuration by " << message().service();
    PolkitQt1::Authority::Result result;
    PolkitQt1::SystemBusNameSubject *subject;

    subject = new PolkitQt1::SystemBusNameSubject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.kde.polkitkde1.changesystemconfiguration",
                                                                      subject, PolkitQt1::Authority::AllowUserInteraction);
    if (result == PolkitQt1::Authority::Yes) {
        qDebug() << "Authorized successfully";
        // It's ok
    } else {
        // It's not ok
        qDebug() << "UnAuthorized! " << PolkitQt1::Authority::instance()->lastError();
        return;
    }

    // Ok, let's see what we have to save here.
    QSettings kdesettings("/etc/polkit-1/polkit-kde-1.conf", QSettings::IniFormat);
    kdesettings.beginGroup("General");

    kdesettings.setValue("ConfigPriority", systemPriority);
    kdesettings.setValue("PoliciesPriority", policiesPriority);

    QString contents = QString("[Configuration]\nAdminIdentities=%1\n").arg(adminIdentities);

    qDebug() << contents << "will be wrote to the local authority file";
    QFile wfile(QString("/etc/polkit-1/localauthority.conf.d/%1-polkitkde.conf").arg(systemPriority));
    wfile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    wfile.write(contents.toUtf8());
    wfile.flush();
    wfile.close();

    // TODO: Move files around if the priority was changed
}

QVariantList PolkitKde1Helper::retrievePolicies()
{
    qDebug() << "Request to retrieve the action authorizations by " << message().service();
    PolkitQt1::Authority::Result result;
    PolkitQt1::SystemBusNameSubject *subject;

    subject = new PolkitQt1::SystemBusNameSubject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.kde.polkitkde1.readauthorizations",
                                                                      subject, PolkitQt1::Authority::AllowUserInteraction);
    if (result == PolkitQt1::Authority::Yes) {
        qDebug() << "Authorized successfully";
        // It's ok
    } else {
        // It's not ok
        qDebug() << "UnAuthorized! " << PolkitQt1::Authority::instance()->lastError();
        return QVariantList();
    }

    QVariantList retlist;

    // Iterate over the directory and find out everything
    QDir baseDir("/var/lib/polkit-1/localauthority/");
    QFileInfoList baseList = baseDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &info, baseList) {
        int filePriority = info.baseName().split('-').first().toInt();
        qDebug() << "Iterating over the directory " << info.absoluteFilePath() << ", which has a priority of " << filePriority;

        QDir nestedDir(info.absoluteFilePath());
        QFileInfoList nestedList = nestedDir.entryInfoList(QDir::Files);

        foreach (const QFileInfo &nestedInfo, nestedList) {
            qDebug() << "Parsing file " << nestedInfo.absoluteFilePath();
            retlist.append(entriesFromFile(filePriority, nestedInfo.absoluteFilePath()));
        }
    }

    return retlist;
}

QVariantList PolkitKde1Helper::entriesFromFile(int filePriority, const QString& filePath)
{
    QList< QVariant > retlist;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open " << filePath;
        return QVariantList();
    }

    int priority = 0;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith('[')) {
            // Ok, that's a key entry
            PKLAEntry entry;
            entry.title = line.split('[').last().split(']').first();
            entry.filePriority = filePriority;
            entry.fileOrder = priority;
            // Now parse over all the rest
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith(QLatin1String("Identity="))) {
                    entry.identity = line.split("Identity=").last();
                } else if (line.startsWith(QLatin1String("Action="))) {
                    entry.action = line.split("Action=").last();
                } else if (line.startsWith(QLatin1String("ResultAny="))) {
                    entry.resultAny = line.split("ResultAny=").last();
                } else if (line.startsWith(QLatin1String("ResultInactive="))) {
                    entry.resultInactive = line.split("ResultInactive=").last();
                } else if (line.startsWith(QLatin1String("ResultActive="))) {
                    entry.resultActive = line.split("ResultActive=").last();
                } else if (line.startsWith('[')) {
                    // Ouch!!
                    qWarning() << "The file appears malformed!! " << filePath;
                    return QVariantList();
                }

                // Did we parse it all?
                if (!entry.identity.isEmpty() && !entry.action.isEmpty() && !entry.resultActive.isEmpty() &&
                    !entry.resultAny.isEmpty() && !entry.resultInactive.isEmpty()) {
                    // Awesome, add and wave goodbye. And also increase the priority
                    ++priority;
                    qDebug() << "PKLA Parsed:" << entry.title << entry.action << entry.identity << entry.resultAny
                             << entry.resultInactive << entry.resultActive << entry.fileOrder;
                    retlist.append(QVariant::fromValue(entry));
                    break;
                }
            }
        }
    }

    return retlist;
}

void PolkitKde1Helper::writePolicy(const QVariantList& policy)
{
    PKLAEntryList entries;
    foreach (const QVariant &variant, policy) {
        entries.append(variant.value<PKLAEntry>());
    }
    qSort(entries.begin(), entries.end(), orderByPriorityLessThan);

    QSettings kdesettings("/etc/polkit-1/polkit-kde-1.conf", QSettings::IniFormat);
    kdesettings.beginGroup("General");

    QString path = QString("/var/lib/polkit-1/localauthority/%1-polkitkde/%2.conf")
                          .arg(kdesettings.value("PoliciesPriority",75).toInt()).arg(entries.first().action);

    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    QString contents;

    foreach (const PKLAEntry &entry, entries) {
        contents.append(formatPKLAEntry(entry));
        contents.append('\n');
    }

    QFile wfile(path);
    wfile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    wfile.write(contents.toUtf8());
    wfile.flush();
    wfile.close();
}

QString PolkitKde1Helper::formatPKLAEntry(const PKLAEntry& entry)
{
    QString retstring;
    retstring.append(QString("[%1]\n").arg(entry.title));
    retstring.append(QString("Identity=%1\n").arg(entry.identity));
    retstring.append(QString("Action=%1\n").arg(entry.action));
    retstring.append(QString("ResultAny=%1\n").arg(entry.resultAny));
    retstring.append(QString("ResultInactive=%1\n").arg(entry.resultInactive));
    retstring.append(QString("ResultActive=%1\n").arg(entry.resultActive));
    return retstring;
}

