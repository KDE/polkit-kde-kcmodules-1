/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "identitywidget.h"

#include "ui_identitywidget.h"
#include <KUser>
#include <KDebug>
#include <QIcon>

IdentityWidget::IdentityWidget(IdentityWidget::IdentityType type, const QString& name, QWidget* parent)
    : QWidget(parent)
{
    init(type);
    setIdentityName(name);
}

IdentityWidget::IdentityWidget(QWidget* parent)
    : QWidget(parent)
{
    init(UserIdentity);
}

IdentityWidget::~IdentityWidget()
{
    delete m_ui;
}

void IdentityWidget::init(IdentityType type)
{
    m_ui = new Ui::IdentityWidget;
    m_ui->setupUi(this);
    m_ui->removeButton->setIcon(QIcon::fromTheme("list-remove"));
    m_ui->identityTypeBox->setItemIcon(0, QIcon::fromTheme("user-identity"));
    m_ui->identityTypeBox->setItemIcon(1, QIcon::fromTheme("system-users"));
    m_ui->identityTypeBox->setCurrentIndex((int)type);
    populateIdentityNameBox();

    connect(m_ui->identityTypeBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(changed()));
    connect(m_ui->identityTypeBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(populateIdentityNameBox()));
    connect(m_ui->identityNameBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(changed()));
    connect(m_ui->removeButton, SIGNAL(clicked(bool)),
            this, SIGNAL(changed()));
    connect(m_ui->removeButton, SIGNAL(clicked(bool)),
            this, SLOT(deleteLater()));
}



QString IdentityWidget::identityName() const
{
    return m_ui->identityNameBox->itemData(m_ui->identityNameBox->currentIndex()).toString();
}

IdentityWidget::IdentityType IdentityWidget::identityType() const
{
    return (IdentityType)(m_ui->identityTypeBox->currentIndex());
}

void IdentityWidget::setIdentityName(const QString& name)
{
    m_ui->identityNameBox->setCurrentIndex(m_ui->identityNameBox->findData(name));
}

void IdentityWidget::setIdentityType(IdentityWidget::IdentityType type)
{
    m_ui->identityTypeBox->setCurrentIndex((int)type);
}

void IdentityWidget::populateIdentityNameBox()
{
    m_ui->identityNameBox->clear();
    if (m_ui->identityTypeBox->currentIndex() == (int)UserIdentity) {
        QList<KUser> users = KUser::allUsers();

        foreach (const KUser &user, users) {
            QIcon icon;
            QString displayName;
            if (!user.faceIconPath().isEmpty()) {
                icon.addPixmap(QPixmap(user.faceIconPath()));
            } else {
                icon = QIcon::fromTheme("user-identity");
            }
            if (user.fullName().isEmpty()) {
                displayName = user.loginName();
            } else {
                displayName = QString("%1 (%2)").arg(user.fullName()).arg(user.loginName());
            }

            m_ui->identityNameBox->addItem(icon, displayName, user.loginName());
        }
    } else {
        QList<KUserGroup> groups = KUserGroup::allGroups();

        foreach (const KUserGroup &group, groups) {
            m_ui->identityNameBox->addItem(QIcon::fromTheme("system-users"), group.name(), group.name());
        }
    }
}
