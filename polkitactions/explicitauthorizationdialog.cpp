/* This file is part of the KDE project

   Copyright (C) 2010 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "explicitauthorizationdialog.h"
#include "ui_explicitwidget.h"
#include "ActionWidget.h"
#include "identitywidget.h"
#include "QIcon"
#include "QDialogButtonBox"

namespace PolkitKde
{

ExplicitAuthorizationDialog::ExplicitAuthorizationDialog(const PKLAEntry &entry, QWidget *parent)
    : QDialog(parent)
    , m_entry(entry)
{
    init();
    m_ui->titleEdit->setEnabled(false);
    reloadPKLA();
}

ExplicitAuthorizationDialog::ExplicitAuthorizationDialog(const QString &action, QWidget *parent)
    : QDialog(parent)
{
    m_entry.action = action;
    m_entry.fileOrder = -1;
    init();
}

ExplicitAuthorizationDialog::~ExplicitAuthorizationDialog()
{
    delete m_ui;
}

void ExplicitAuthorizationDialog::init()
{
    auto *widget = new QWidget;
    m_ui = new Ui::ExplicitAuthorizationWidget;
    m_ui->setupUi(widget);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(widget);
    mainLayout->addWidget(buttonBox);

    setModal(true);

    m_ui->addButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

    m_identitiesLayout = new QVBoxLayout;
    m_identitiesLayout->addStretch();
    m_ui->scrollAreaWidgetContents->setLayout(m_identitiesLayout);
    connect(m_ui->addButton, &QAbstractButton::clicked, this, &ExplicitAuthorizationDialog::addIdentity);
}

void ExplicitAuthorizationDialog::addIdentity()
{
    auto *iw = new IdentityWidget();
    m_identitiesLayout->insertWidget(m_identitiesLayout->count() - 1, iw);
}

void ExplicitAuthorizationDialog::reloadPKLA()
{
    m_ui->titleEdit->setText(m_entry.title);
    m_ui->anyComboBox->setCurrentIndex(ActionWidget::comboBoxIndexFor(PKLAEntry::implFromText(m_entry.resultAny)));
    m_ui->inactiveComboBox->setCurrentIndex(ActionWidget::comboBoxIndexFor(PKLAEntry::implFromText(m_entry.resultInactive)));
    m_ui->activeComboBox->setCurrentIndex(ActionWidget::comboBoxIndexFor(PKLAEntry::implFromText(m_entry.resultActive)));

    foreach (const QString & identity, m_entry.identity.split(QLatin1Char(';'))) {
        IdentityWidget *idWidget = nullptr;
        if (identity.startsWith(QLatin1String("unix-user:"))) {
            idWidget = new IdentityWidget(IdentityWidget::UserIdentity, identity.split(QStringLiteral("unix-user:")).last());
        } else if (identity.startsWith(QLatin1String("unix-group:"))) {
            idWidget = new IdentityWidget(IdentityWidget::GroupIdentity, identity.split(QStringLiteral("unix-group:")).last());
        }

        if (idWidget) {
            m_identitiesLayout->insertWidget(m_identitiesLayout->count() - 1, idWidget);
        }
    }
}

void ExplicitAuthorizationDialog::commitChangesToPKLA()
{
    m_entry.title = m_ui->titleEdit->text();
    m_entry.resultAny = PKLAEntry::textFromImpl(ActionWidget::implicitAuthorizationFor(m_ui->anyComboBox->currentIndex()));
    m_entry.resultActive = PKLAEntry::textFromImpl(ActionWidget::implicitAuthorizationFor(m_ui->activeComboBox->currentIndex()));
    m_entry.resultInactive =
        PKLAEntry::textFromImpl(ActionWidget::implicitAuthorizationFor(m_ui->inactiveComboBox->currentIndex()));

    QString identities;
    for (int i = 0; i < m_identitiesLayout->count(); ++i) {
        QLayoutItem *item = m_identitiesLayout->itemAt(i);
        if (item != nullptr) {
            QWidget *widget = item->widget();
            if (widget != nullptr) {
                auto *identityWidget = qobject_cast<IdentityWidget *>(widget);
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

    m_entry.identity = identities;
}

PKLAEntry ExplicitAuthorizationDialog::pkla()
{
    return m_entry;
}

}
