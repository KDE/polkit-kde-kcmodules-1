/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KCMPOLKITCONFIG_H
#define KCMPOLKITCONFIG_H

#include <kcmodule.h>

class QVBoxLayout;
namespace Ui
{
class PolkitConfig;
}

class KCMPolkitConfig : public KCModule
{
    Q_OBJECT
public:
    KCMPolkitConfig(QWidget *parent, const QVariantList &args);
    virtual ~KCMPolkitConfig();

    virtual void defaults();
    virtual void load();
    virtual void save();

private slots:
    void addNewIdentity();

private:
    Ui::PolkitConfig *m_ui;
    QVBoxLayout *m_identitiesLayout;
};

#endif // KCMPOLKITCONFIG_H
