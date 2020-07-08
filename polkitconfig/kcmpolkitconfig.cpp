/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

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
#include <qlayoutitem.h>
#include "identitywidget.h"
#include <QIcon>
#include <QDebug>

K_PLUGIN_FACTORY(KCMPolkitConfigFactory,
                 registerPlugin<KCMPolkitConfig>();
                )

#include <kcmpolkitconfig.moc>

KCMPolkitConfig::KCMPolkitConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    KAboutData *about =
        new KAboutData(QStringLiteral("kcm_polkitconfig"), QStringLiteral("kcm_polkitconfig"),
                       QStringLiteral("1.0.0"), i18n("A configuration for polkit-1 system administrators and policy priorities"),
                       KAboutLicense::GPL, i18n("(c), 2009 Dario Freddi"),
                       i18n("From this module, you can configure system administrators and priorities "
                            "for the policies defined in the Actions module"));

    about->addAuthor(i18n("Dario Freddi"), i18n("Maintainer"), QStringLiteral("drf@kde.org"), QStringLiteral("http://drfav.wordpress.com"));

    setAboutData(about);

    m_ui = new Ui::PolkitConfig;
    m_ui->setupUi(this);

    m_ui->warningTextLabel->setVisible(false);
    m_ui->warningPixmapLabel->setVisible(false);
    m_ui->addIdentityButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

    m_identitiesLayout = new QVBoxLayout;
    m_identitiesLayout->addStretch();
    m_ui->scrollAreaWidgetContents->setLayout(m_identitiesLayout);

    connect(m_ui->addIdentityButton, SIGNAL(clicked(bool)),
            this, SLOT(addNewIdentity()));
    connect(m_ui->configPrioritySpin, SIGNAL(valueChanged(int)),
            this, SLOT(changed()));
    connect(m_ui->policyPrioritySpin, SIGNAL(valueChanged(int)),
            this, SLOT(changed()));
}

KCMPolkitConfig::~KCMPolkitConfig()
{
    delete m_ui;
}

void KCMPolkitConfig::defaults()
{

}

void KCMPolkitConfig::load()
{
    // Load the first tab
    QSettings settings(QStringLiteral("/etc/polkit-1/polkit-kde-1.conf"), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("General"));

    int priority = settings.value(QStringLiteral("ConfigPriority"), 75).toInt();
    int highestPriority = -1;
    QString highestFilename;

    QDir dir(QStringLiteral("/etc/polkit-1/localauthority.conf.d/"));
    dir.setFilter(QDir::Files);
    QFileInfoList infolist = dir.entryInfoList();

    for (const QFileInfo &finfo : qAsConst(infolist)) {
        int fpriority = finfo.baseName().split(QLatin1Char('-')).first().toInt();
        qDebug() << "Considering " << finfo.absoluteFilePath() << " which should have priority " << fpriority;
        if (fpriority > highestPriority) {
            qDebug() << "Setting it as highest priority";
            highestPriority = fpriority;
            highestFilename = finfo.absoluteFilePath();
        }
    }

    if (highestPriority > priority) {
        qDebug() << "Highest priority is " << highestPriority << ", polkit kde priority is " << priority;
        m_ui->warningTextLabel->setText(i18n("The changes will have no effect, since another policy has an higher priority "
                                             "(%1). Please change the priority of policies defined through this module to an "
                                             "higher value.", highestPriority));
        m_ui->warningTextLabel->setVisible(true);
        m_ui->warningPixmapLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(48));
        m_ui->warningPixmapLabel->setVisible(true);
    }

    qDebug() << "The highest filename is " << highestFilename;
    QFile policyFile(highestFilename);
    policyFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString identities = QString::fromLocal8Bit(policyFile.readAll()).split(QStringLiteral("AdminIdentities=")).last();
    identities = identities.split(QLatin1Char('\n')).first();
    policyFile.close();

    qDebug() << "our identities are " << identities;
    foreach (const QString & identity, identities.split(QLatin1Char(';'))) {
        IdentityWidget::IdentityType type;
        if (identity.split(QLatin1Char(':')).first() == QLatin1String("unix-user")) {
            type = IdentityWidget::UserIdentity;
            qDebug() << "It's an user";
        } else {
            type = IdentityWidget::GroupIdentity;
            qDebug() << "It's a group";
        }
        QString name = identity.split(QLatin1Char(':')).last();
        IdentityWidget *iw = new IdentityWidget(type, name);
        m_identitiesLayout->insertWidget(m_identitiesLayout->count() - 1, iw);
        connect(iw, SIGNAL(changed()), this, SLOT(changed()));
    }

    // Set up the other tab
    m_ui->configPrioritySpin->setValue(priority);
    m_ui->policyPrioritySpin->setValue(settings.value(QStringLiteral("PoliciesPriority"), 75).toInt());
}

void KCMPolkitConfig::save()
{
    // Get back all the identities first
    QString identities;
    for (int i = 0; i < m_identitiesLayout->count(); ++i) {
        QLayoutItem *item = m_identitiesLayout->itemAt(i);
        if (item != nullptr) {
            QWidget *widget = item->widget();
            if (widget != nullptr) {
                IdentityWidget *identityWidget = qobject_cast<IdentityWidget *>(widget);
                if (identityWidget != nullptr) {
                    // Whew, let's add it
                    if (identityWidget->identityType() == IdentityWidget::UserIdentity) {
                        identities.append(QStringLiteral("unix-user:"));
                    } else {
                        identities.append(QStringLiteral("unix-group:"));
                    }
                    identities.append(identityWidget->identityName());
                    identities.append(QLatin1Char(';'));
                }
            }
        }
    }

    if (!identities.isEmpty()) {
        identities.remove(identities.length() - 1, 1);
    }

    qDebug() << "Identities to save: " << identities;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.polkitkde1.helper"),
                                                          QStringLiteral("/Helper"),
                                                          QStringLiteral("org.kde.polkitkde1.helper"),
                                                          QStringLiteral("saveGlobalConfiguration"));
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(identities);
    argumentList << QVariant::fromValue(m_ui->configPrioritySpin->value());
    argumentList << QVariant::fromValue(m_ui->policyPrioritySpin->value());
    message.setArguments(argumentList);

    QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message);
}

void KCMPolkitConfig::addNewIdentity()
{
    IdentityWidget *iw = new IdentityWidget();
    m_identitiesLayout->insertWidget(m_identitiesLayout->count() - 1, iw);
    connect(iw, SIGNAL(changed()), this, SLOT(changed()));
    changed();
}

