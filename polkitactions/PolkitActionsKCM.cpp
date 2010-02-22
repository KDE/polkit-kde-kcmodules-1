/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "PolkitActionsKCM.h"

#include <KPluginFactory>
#include <KAboutData>
#include <klocalizedstring.h>
#include <PolkitQt1/Authority>
#include "ui_mainview.h"
#include "PoliciesModel.h"
#include "AuthorizationsFilterModel.h"
#include "pkitemdelegate.h"
#include <QLayout>
#include "ActionWidget.h"
#include "PolicyItem.h"
#include <QDBusMessage>
#include <QDBusConnection>
#include <qdbuspendingcall.h>

K_PLUGIN_FACTORY(KCMPolkitActionsFactory,
                 registerPlugin<PolkitActionsKCM>();
                )
K_EXPORT_PLUGIN(KCMPolkitActionsFactory("kcm_polkitactions"))

PolkitActionsKCM::PolkitActionsKCM(QWidget* parent, const QVariantList& args)
    : KCModule(KCMPolkitActionsFactory::componentData(), parent, args)
    , m_ui(new Ui::PolkitActionsMainView)
{
    KAboutData *about =
        new KAboutData("kcm_polkitactions", "kcm_polkitactions", ki18n("Global system policy settings"),
                       "1.0.0", ki18n("A configuration for polkit-1 system administrators and policy priorities"),
                       KAboutData::License_GPL, ki18n("(c), 2009 Dario Freddi"),
                       ki18n("From this module, you can configure system administrators and priorities "
                             "for the policies defined in the Actions module"));

    about->addAuthor(ki18n("Dario Freddi"), ki18n("Maintainer") , "drf@kde.org", "http://drfav.wordpress.com");

    setAboutData(about);

    // Build the UI
    m_ui->setupUi(this);
    m_model = new PolkitKde::PoliciesModel(this);
    m_proxyModel = new PolkitKde::AuthorizationsFilterModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_ui->treeView->header()->hide();
    m_ui->treeView->setModel(m_proxyModel);
    m_ui->treeView->setItemDelegate(new PolkitKde::PkItemDelegate(this));

    connect(m_ui->searchLine, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterRegExp(QString)));
    connect(m_ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)));

    // Initialize
    connect(PolkitQt1::Authority::instance(), SIGNAL(checkAuthorizationFinished(PolkitQt1::Authority::Result)),
            this, SLOT(slotCheckAuthorizationFinished(PolkitQt1::Authority::Result)));
    connect(PolkitQt1::Authority::instance(), SIGNAL(enumerateActionsFinished(PolkitQt1::ActionDescription::List)),
            m_model, SLOT(setCurrentEntries(PolkitQt1::ActionDescription::List)));
    PolkitQt1::Authority::instance()->enumerateActions();
}

PolkitActionsKCM::~PolkitActionsKCM()
{

}

void PolkitActionsKCM::load()
{
    KCModule::load();
}

void PolkitActionsKCM::save()
{
    if (m_actionWidget.isNull()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.kde.polkitkde1.helper",
                                                          "/Helper",
                                                          "org.kde.polkitkde1.helper",
                                                          QLatin1String("writePolicy"));
    QList<QVariant> argumentList;
    QVariantList policies;
    foreach (const PKLAEntry &entry, m_actionWidget.data()->entries()) {
        policies << QVariant::fromValue(entry);
    }
    argumentList << policies;

    message.setArguments(argumentList);

    QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message);
}

void PolkitActionsKCM::defaults()
{
    KCModule::defaults();
}

void PolkitActionsKCM::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{

}

void PolkitActionsKCM::slotCurrentChanged(const QModelIndex& current, const QModelIndex& )
{
    if (current.data(PolkitKde::PoliciesModel::IsGroupRole).toBool() == false) {
        PolkitQt1::ActionDescription *action;
        action = current.data(PolkitKde::PoliciesModel::PolkitEntryRole).value<PolkitQt1::ActionDescription *>();

        if (action) {
            if (m_actionWidget.isNull()) {
                if (layout()->count() == 2) {
                    layout()->takeAt(1)->widget()->deleteLater();
                }
                m_actionWidget = new PolkitKde::ActionWidget(action);
                connect(m_actionWidget, SIGNAL(changed()), this, SLOT(changed()));
                layout()->addWidget(m_actionWidget.data());
            } else {
                m_actionWidget.data()->setAction(action);
            }
        }
    } else {
        // TODO
    }
}

