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

PolkitKde1Helper::PolkitKde1Helper(QObject* parent)
    : QObject(parent)
{
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
    PolkitQt1::Authority::Result result;
    PolkitQt1::SystemBusNameSubject *subject;

    subject = new PolkitQt1::SystemBusNameSubject(message().service());

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.kde.polkitkde1.changesystemconfiguration",
                                                                      subject, PolkitQt1::Authority::AllowUserInteraction);
    if (result == PolkitQt1::Authority::Yes) {
        // It's ok
    } else {
        // It's not ok
        return;
    }

    // Ok, let's see what we have to save here.
    QSettings kdesettings("/etc/polkit-1/polkit-kde-1.conf", QSettings::IniFormat);
    kdesettings.beginGroup("General");

    kdesettings.setValue("ConfigPriority", systemPriority);
    kdesettings.setValue("PoliciesPriority", policiesPriority);

    QSettings polkitsettings(QString("/etc/polkit-1/localauthority.conf.d/%1-polkitkde.conf").arg(systemPriority),
                             QSettings::IniFormat);
    polkitsettings.beginGroup("Configuration");
    polkitsettings.setValue("AdminIdentities", adminIdentities);

    // TODO: Move files around if the priority was changed
}

