/* This file is part of the KDE project

   Copyright (C) 2008 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kcmpolkitconfig.h"

#include "ui_polkitconfig.h"

#include <KPluginFactory>
#include <KAboutData>
#include <klocalizedstring.h>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnectionInterface>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qdir.h>
#include "identitywidget.h"
#include <KDebug>

K_PLUGIN_FACTORY(KCMPolkitConfigFactory,
                 registerPlugin<KCMPolkitConfig>();
                )
K_EXPORT_PLUGIN(KCMPolkitConfigFactory("kcm_polkitconfig"))

KCMPolkitConfig::KCMPolkitConfig(QWidget* parent, const QVariantList& args)
    : KCModule(KCMPolkitConfigFactory::componentData(), parent, args)
{
    KAboutData *about =
        new KAboutData("kcm_polkitconfig", "kcm_polkitconfig", ki18n("Global system policy settings"),
                       "1.0.0", ki18n("A configuration for polkit-1 system administrators and policy priorities"),
                       KAboutData::License_GPL, ki18n("(c), 2009 Dario Freddi"),
                       ki18n("From this module, you can configure system administrators and priorities "
                             "for the policies defined in the Actions module"));

    about->addAuthor(ki18n("Dario Freddi"), ki18n("Maintainer") , "drf@kde.org",
                     "http://drfav.wordpress.com");

    setAboutData(about);

    m_ui = new Ui::PolkitConfig;
    m_ui->setupUi(this);

    m_ui->warningTextLabel->setVisible(false);
    m_ui->warningPixmapLabel->setVisible(false);
    m_ui->addIdentityButton->setIcon(KIcon("list-add"));

    m_identitiesLayout = new QVBoxLayout;
    m_identitiesLayout->addStretch();
    m_ui->scrollAreaWidgetContents->setLayout(m_identitiesLayout);

    connect(m_ui->addIdentityButton, SIGNAL(clicked(bool)),
            this, SLOT(addNewIdentity()));
}

KCMPolkitConfig::~KCMPolkitConfig()
{

}

void KCMPolkitConfig::defaults()
{

}

void KCMPolkitConfig::load()
{
    // Load the first tab
    QSettings settings("/etc/polkit-1/polkit-kde-1.conf", QSettings::IniFormat);
    settings.beginGroup("General");

    int priority = settings.value("ConfigPriority", 40).toInt();
    int highestPriority = -1;
    QString highestFilename;

    QDir dir("/etc/polkit-1/localauthority.conf.d/");
    dir.setFilter(QDir::Files);
    QFileInfoList infolist = dir.entryInfoList();

    foreach (const QFileInfo &finfo, infolist) {
        int fpriority = finfo.baseName().split('-').first().toInt();
        kDebug() << "Considering " << finfo.absoluteFilePath() << " which should have priority "<< fpriority;
        if (fpriority > highestPriority) {
            kDebug() << "Setting it as highest priority";
            highestPriority = fpriority;
            highestFilename = finfo.absoluteFilePath();
        }
    }

    if (highestPriority > priority) {
        m_ui->warningTextLabel->setText(i18n("The changes will have no effect, since another policy has an higher priority "
                                             "(%1). Please change the priority of policies defined through this module to an "
                                             "higher value."));
        m_ui->warningTextLabel->setVisible(true);
        m_ui->warningPixmapLabel->setPixmap(KIcon("dialog-warning").pixmap(48));
        m_ui->warningPixmapLabel->setVisible(true);
    }

    kDebug() << "The highest filename is " << highestFilename;
    QSettings policy(highestFilename, QSettings::IniFormat);
    kDebug() << policy.allKeys();
    kDebug() << policy.childGroups();
    policy.beginGroup("Configuration");

    QString identities = policy.value("AdminIdentities").toString();
    kDebug() << "our identities are " << identities;
    foreach (const QString &identity, identities.split(';')) {
        IdentityWidget::IdentityType type;
        if (identity.split(':').first() == "unix-user") {
            type = IdentityWidget::UserIdentity;
            kDebug() << "It's an user";
        } else {
            type = IdentityWidget::GroupIdentity;
            kDebug() << "It's a group";
        }
        QString name = identity.split(':').last();
        IdentityWidget *iw = new IdentityWidget(type, name);
        m_identitiesLayout->insertWidget(m_identitiesLayout->count() - 1, iw);
    }

    // Set up the other tab
    m_ui->configPrioritySpin->setValue(priority);
    m_ui->policyPrioritySpin->setValue(settings.value("PolicyPriority", 75).toInt());
}

void KCMPolkitConfig::save()
{
KCModule::save();
}

void KCMPolkitConfig::addNewIdentity()
{
    m_identitiesLayout->insertWidget(m_identitiesLayout->count() - 1, new IdentityWidget());
}

