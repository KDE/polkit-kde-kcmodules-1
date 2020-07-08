/* This file is part of the KDE project

   Copyright (C) 2010 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef EXPLICITAUTHORIZATIONDIALOG_H
#define EXPLICITAUTHORIZATIONDIALOG_H

#include <QDialog>
#include "PKLAEntry.h"

namespace Ui
{
class ExplicitAuthorizationWidget;
}
class QVBoxLayout;

namespace PolkitKde
{

class ExplicitAuthorizationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExplicitAuthorizationDialog(const PKLAEntry &entry, QWidget *parent = nullptr);
    explicit ExplicitAuthorizationDialog(const QString &action, QWidget *parent = nullptr);
    virtual ~ExplicitAuthorizationDialog();

    void commitChangesToPKLA();
    PKLAEntry pkla();

private Q_SLOTS:
    void addIdentity();

private:
    void init();
    void reloadPKLA();

private:
    PKLAEntry m_entry;
    Ui::ExplicitAuthorizationWidget *m_ui;
    QVBoxLayout *m_identitiesLayout;
};

}

#endif // EXPLICITAUTHORIZATIONDIALOG_H
