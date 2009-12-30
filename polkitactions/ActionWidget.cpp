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
#include <QDebug>

namespace PolkitKde {

ActionWidget::ActionWidget(PolkitQt1::ActionDescription* action, QWidget* parent)
        : QWidget(parent)
        , m_ui(new Ui::ActionWidget)
        , m_action(0)
{
    m_ui->setupUi(this);

    // Initialize
    reloadPKLAs();

    setAction(action);
}

ActionWidget::~ActionWidget()
{

}

void ActionWidget::reloadPKLAs()
{
    m_entries.clear();
    QDBusMessage message = QDBusMessage::createMethodCall("org.kde.polkitkde1.helper",
                                                          "/Helper",
                                                          "org.kde.polkitkde1.helper",
                                                          QLatin1String("retrievePolicies"));
    QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message);
    reply.waitForFinished();
    if (reply.reply().arguments().count() >= 1) {
        QVariantList vlist;
        reply.reply().arguments().first().value<QDBusArgument>() >> vlist;
        foreach (const QVariant &variant, vlist) {
            PKLAEntry entry;
            variant.value<QDBusArgument>() >> entry;
            qDebug() << entry.title;
            m_entries.append(entry);
        }
    }

    if (m_action) {
        computeActionPolicies();
    }
}

void ActionWidget::computeActionPolicies()
{
    foreach (const PKLAEntry &entry, m_entries) {
        QStringList realActions = entry.action.split(';');
        if (realActions.contains(m_action->actionId())) {
            // Match! Is it, actually, an implicit override?
            if (entry.title == "PolkitKdeOverrideImplicit") {
                // It is!
                setImplicitAuthorization(implFromText(entry.resultActive), m_ui->activeComboBox);
                setImplicitAuthorization(implFromText(entry.resultInactive), m_ui->inactiveComboBox);
                setImplicitAuthorization(implFromText(entry.resultAny), m_ui->anyComboBox);
            } else {
                // TODO: Add it to the local auths
            }
        }
    }
}

PolkitQt1::ActionDescription::ImplicitAuthorization ActionWidget::implFromText(const QString& text)
{
    if (text == "yes") {
        return PolkitQt1::ActionDescription::Authorized;
    } else if (text == "no") {
        return PolkitQt1::ActionDescription::NotAuthorized;
    } else if (text == "auth_admin") {
        return PolkitQt1::ActionDescription::AdministratorAuthenticationRequired;
    } else if (text == "auth_admin_keep") {
        return PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained;
    } else if (text == "auth_self") {
        return PolkitQt1::ActionDescription::AuthenticationRequired;
    } else if (text == "auth_self_keep") {
        return PolkitQt1::ActionDescription::AuthenticationRequiredRetained;
    } else {
        return PolkitQt1::ActionDescription::Unknown;
    }
}

void ActionWidget::setImplicitAuthorization(PolkitQt1::ActionDescription::ImplicitAuthorization auth, QComboBox* box)
{
    switch (auth) {
        case PolkitQt1::ActionDescription::Authorized:
            box->setCurrentIndex(0);
            break;
        case PolkitQt1::ActionDescription::NotAuthorized:
            box->setCurrentIndex(1);
            break;
        case PolkitQt1::ActionDescription::AuthenticationRequired:
            box->setCurrentIndex(4);
            break;
        case PolkitQt1::ActionDescription::AuthenticationRequiredRetained:
            box->setCurrentIndex(5);
            break;
        case PolkitQt1::ActionDescription::AdministratorAuthenticationRequired:
            box->setCurrentIndex(2);
            break;
        case PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained:
            box->setCurrentIndex(3);
            break;
    }
}

void ActionWidget::setAction(PolkitQt1::ActionDescription* action)
{
    m_action = action;
    setImplicitAuthorization(action->implicitActive(), m_ui->activeComboBox);
    setImplicitAuthorization(action->implicitInactive(), m_ui->inactiveComboBox);
    setImplicitAuthorization(action->implicitAny(), m_ui->anyComboBox);

    m_ui->descriptionLabel->setText(action->description());
    m_ui->vendorLabel->setText(action->vendorName());
    m_ui->vendorLabel->setUrl(action->vendorUrl());
    m_ui->pixmapLabel->setPixmap(KIcon(action->iconName()).pixmap(64));

    computeActionPolicies();
}

}
