/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "ActionWidget.h"

#include "ui_actionwidget.h"

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <qdbuspendingcall.h>
#include <QtDBus/qdbusmetatype.h>

namespace PolkitKde {

ActionWidget::ActionWidget(PolkitQt1::ActionDescription* action, QWidget* parent)
        : QWidget(action, parent)
        , m_ui(new Ui::ActionWidget)
        , m_action(action)
{
    m_ui->setupUi(this);

    qDBusRegisterMetaType<PKLAEntry>();
    QDBusConnection::systemBus().connect("org.kde.polkitkde1.helper", "/Helper", "org.kde.polkitkde1.helper",
                                         "policiesRetrieved", this, SLOT(policiesRetrieved(PKLAEntryList)));
    QDBusMessage message = QDBusMessage::createMethodCall("org.kde.polkitkde1.helper",
                                                          "/Helper",
                                                          "org.kde.polkitkde1.helper",
                                                          QLatin1String("retrievePolicies"));
    QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message);
}

ActionWidget::~ActionWidget()
{

}

}
