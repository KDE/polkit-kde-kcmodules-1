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
#include "../polkitconfig/identitywidget.h"

namespace PolkitKde {

ExplicitAuthorizationDialog::ExplicitAuthorizationDialog(const PKLAEntry& entry, QWidget* parent)
        : KDialog(parent)
        , m_entry(entry)
{
    init();
    reloadPKLA();
}

ExplicitAuthorizationDialog::ExplicitAuthorizationDialog(QWidget* parent)
        : KDialog(parent)
{
    init();
    reloadPKLA();
}

ExplicitAuthorizationDialog::~ExplicitAuthorizationDialog()
{

}

void ExplicitAuthorizationDialog::init()
{
    QWidget *widget = new QWidget;
    m_ui = new Ui::ExplicitAuthorizationWidget;
    m_ui->setupUi(widget);
    setMainWidget(widget);
    setModal(true);

    m_identitiesLayout = new QVBoxLayout;
    m_identitiesLayout->addStretch();
    m_ui->scrollAreaWidgetContents->setLayout(m_identitiesLayout);
}

void ExplicitAuthorizationDialog::reloadPKLA()
{
    m_ui->titleEdit->setText(m_entry.title);
    m_ui->anyComboBox->setCurrentIndex(ActionWidget::comboBoxIndexFor(PKLAEntry::implFromText(m_entry.resultAny)));
    m_ui->inactiveComboBox->setCurrentIndex(ActionWidget::comboBoxIndexFor(PKLAEntry::implFromText(m_entry.resultInactive)));
    m_ui->activeComboBox->setCurrentIndex(ActionWidget::comboBoxIndexFor(PKLAEntry::implFromText(m_entry.resultActive)));

    foreach (const QString &identity, m_entry.identity.split(';')) {
        IdentityWidget *idWidget = 0;
        if (identity.startsWith("unix-user:")) {
            idWidget = new IdentityWidget(IdentityWidget::UserIdentity, identity.split("unix-user:").last());
        } else if (identity.startsWith("unix-group:")) {
            idWidget = new IdentityWidget(IdentityWidget::GroupIdentity, identity.split("unix-group:").last());
        }

        if (idWidget) {
            m_identitiesLayout->insertWidget(m_identitiesLayout->count() - 1, idWidget);
        }
    }
}

}
