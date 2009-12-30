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
#include <PolkitQt1/ActionDescription>

namespace PolkitKde {

ActionWidget::ActionWidget(PolkitQt1::ActionDescription* action, QWidget* parent)
        : QWidget(parent)
        , m_ui(new Ui::ActionWidget)
        , m_action(action)
{
    m_ui->setupUi(this);

    // Initialize
    switch (action->implicitActive()) {
        case PolkitQt1::ActionDescription::Authorized:
            m_ui->activeComboBox->setCurrentIndex(0);
            break;
        case PolkitQt1::ActionDescription::NotAuthorized:
            m_ui->activeComboBox->setCurrentIndex(1);
            break;
        case PolkitQt1::ActionDescription::AuthenticationRequired:
            m_ui->activeComboBox->setCurrentIndex(4);
            break;
        case PolkitQt1::ActionDescription::AuthenticationRequiredRetained:
            m_ui->activeComboBox->setCurrentIndex(5);
            break;
        case PolkitQt1::ActionDescription::AdministratorAuthenticationRequired:
            m_ui->activeComboBox->setCurrentIndex(2);
            break;
        case PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained:
            m_ui->activeComboBox->setCurrentIndex(3);
            break;
    }
    switch (action->implicitInactive()) {
        case PolkitQt1::ActionDescription::Authorized:
            m_ui->inactiveComboBox->setCurrentIndex(0);
            break;
        case PolkitQt1::ActionDescription::NotAuthorized:
            m_ui->inactiveComboBox->setCurrentIndex(1);
            break;
        case PolkitQt1::ActionDescription::AuthenticationRequired:
            m_ui->inactiveComboBox->setCurrentIndex(4);
            break;
        case PolkitQt1::ActionDescription::AuthenticationRequiredRetained:
            m_ui->inactiveComboBox->setCurrentIndex(5);
            break;
        case PolkitQt1::ActionDescription::AdministratorAuthenticationRequired:
            m_ui->inactiveComboBox->setCurrentIndex(2);
            break;
        case PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained:
            m_ui->inactiveComboBox->setCurrentIndex(3);
            break;
    }
    switch (action->implicitAny()) {
        case PolkitQt1::ActionDescription::Authorized:
            m_ui->anyComboBox->setCurrentIndex(0);
            break;
        case PolkitQt1::ActionDescription::NotAuthorized:
            m_ui->anyComboBox->setCurrentIndex(1);
            break;
        case PolkitQt1::ActionDescription::AuthenticationRequired:
            m_ui->anyComboBox->setCurrentIndex(4);
            break;
        case PolkitQt1::ActionDescription::AuthenticationRequiredRetained:
            m_ui->anyComboBox->setCurrentIndex(5);
            break;
        case PolkitQt1::ActionDescription::AdministratorAuthenticationRequired:
            m_ui->anyComboBox->setCurrentIndex(2);
            break;
        case PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained:
            m_ui->anyComboBox->setCurrentIndex(3);
            break;
    }
    m_ui->descriptionLabel->setText(action->description());
    m_ui->vendorLabel->setText(action->vendorName());
    m_ui->vendorLabel->setUrl(action->vendorUrl());
    m_ui->pixmapLabel->setPixmap(KIcon(action->iconName()).pixmap(64));

    qDBusRegisterMetaType<PKLAEntry>();
    QDBusConnection::systemBus().connect("org.kde.polkitkde1.helper", "/Helper", "org.kde.polkitkde1.helper",
                                         "policiesRetrieved", this, SLOT(slotPoliciesRetrieved(PKLAEntryList)));
    QDBusMessage message = QDBusMessage::createMethodCall("org.kde.polkitkde1.helper",
                                                          "/Helper",
                                                          "org.kde.polkitkde1.helper",
                                                          QLatin1String("retrievePolicies"));
    QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message);
}

ActionWidget::~ActionWidget()
{

}

void ActionWidget::slotPoliciesRetrieved(const PKLAEntryList& policies)
{
}

}
