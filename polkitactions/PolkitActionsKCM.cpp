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
#include <KLocalizedString>
#include <KMessageBox>
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
#include <QDBusMetaType>
#include <QDBusPendingCall>

K_PLUGIN_FACTORY(KCMPolkitActionsFactory,
                 registerPlugin<PolkitActionsKCM>();
                )

#include <PolkitActionsKCM.moc>

PolkitActionsKCM::PolkitActionsKCM(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_ui(new Ui::PolkitActionsMainView)
{
    KAboutData *about =
        new KAboutData(QStringLiteral("kcm_polkitactions"), QStringLiteral("kcm_polkitactions"),
                       QStringLiteral("1.0.0"), i18n("A configuration for polkit-1 system administrators and policy priorities"),
                       KAboutLicense::GPL, i18n("(c), 2009 Dario Freddi"),
                       i18n("From this module, you can configure system administrators and priorities "
                            "for the policies defined in the Actions module"));

    about->addAuthor(i18n("Dario Freddi"), i18n("Maintainer"), QStringLiteral("drf@kde.org"), QStringLiteral("http://drfav.wordpress.com"));

    setAboutData(about);

    qRegisterMetaType<PolkitQt1::ActionDescription>();
    qRegisterMetaType<PKLAEntry>("PKLAEntry");
    qDBusRegisterMetaType<PKLAEntry>();
    qDBusRegisterMetaType<QList<PKLAEntry> >();

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

    // Initialize ActionWidget
    if (layout()->count() == 2) {
        layout()->takeAt(1)->widget()->deleteLater();
    }
    m_actionWidget = new PolkitKde::ActionWidget(this);
    connect(m_actionWidget, SIGNAL(changed()), this, SLOT(changed()));
    connect(this, SIGNAL(implicitSaved()), m_actionWidget, SLOT(implicitSettingsSaved()));
    connect(this, SIGNAL(explicitSaved()), m_actionWidget, SLOT(explicitSettingsSaved()));
    layout()->addWidget(m_actionWidget);
}

PolkitActionsKCM::~PolkitActionsKCM()
{
    delete m_ui;
}

void PolkitActionsKCM::load()
{
    KCModule::load();
}

void PolkitActionsKCM::save()
{
    // HACK: Check if we want to save the implicit settings. This will be changed
    // in the future, where implicitEntries() will only contain the changed settings.
    if (m_actionWidget->isImplicitSettingsChanged()) {
        QDBusMessage messageImplicit = QDBusMessage::createMethodCall(QStringLiteral("org.kde.polkitkde1.helper"),
                                                                      QStringLiteral("/Helper"),
                                                                      QStringLiteral("org.kde.polkitkde1.helper"),
                                                                      QStringLiteral("writeImplicitPolicy"));
        QList<QVariant> implicitArgumentList;
        implicitArgumentList << QVariant::fromValue(m_actionWidget->implicitEntries());

        messageImplicit.setArguments(implicitArgumentList);

        QDBusPendingCall replyImplicit = QDBusConnection::systemBus().asyncCall(messageImplicit);
        replyImplicit.waitForFinished();
        if (replyImplicit.isError()) {
            KMessageBox::detailedError(this,
                                       replyImplicit.error().name(),
                                       replyImplicit.error().message()
                                      );
            changed();
        } else {
            emit implicitSaved();
        }
    }

    // HACK: Check if we want to save the explicit settings. This will be changed
    // in the future, where entries() will only contain the changed settings.
    if (m_actionWidget->isExplicitSettingsChanged()) {
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.polkitkde1.helper"),
                                                              QStringLiteral("/Helper"),
                                                              QStringLiteral("org.kde.polkitkde1.helper"),
                                                              QStringLiteral("writePolicy"));
        QList<QVariant> argumentList;
        QList<PKLAEntry> policies;
        foreach (const PKLAEntry &entry, m_actionWidget->entries()) {
            policies << entry;
        }
        argumentList << QVariant::fromValue(policies);

        message.setArguments(argumentList);

        QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message);
        if (reply.isError()) {
            KMessageBox::detailedError(this,
                                       reply.error().name(),
                                       reply.error().message()
                                      );
            changed();
        } else {
            emit explicitSaved();
        }
    }
}

void PolkitActionsKCM::defaults()
{
    KCModule::defaults();
}

void PolkitActionsKCM::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{

}

void PolkitActionsKCM::slotCurrentChanged(const QModelIndex &current, const QModelIndex &)
{
    if (current.data(PolkitKde::PoliciesModel::IsGroupRole).toBool() == false) {
        PolkitQt1::ActionDescription action;
        action = current.data(PolkitKde::PoliciesModel::PolkitEntryRole).value<PolkitQt1::ActionDescription>();

        m_actionWidget->setAction(action);
    } else {
        // TODO
    }
}

