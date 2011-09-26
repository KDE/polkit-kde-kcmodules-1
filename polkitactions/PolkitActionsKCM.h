/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef POLKITACTIONSKCM_H
#define POLKITACTIONSKCM_H

#include <kcmodule.h>
#include <PolkitQt1/Authority>
#include <QtCore/QPointer>

class QModelIndex;
namespace PolkitKde {
class PoliciesModel;
class AuthorizationsFilterModel;
class ActionWidget;
}

namespace Ui {
    class PolkitActionsMainView;
}

class PolkitActionsKCM : public KCModule
{
    Q_OBJECT
    public:
        explicit PolkitActionsKCM(QWidget* parent = 0, const QVariantList& args = QVariantList());
        virtual ~PolkitActionsKCM();

        virtual void load();
        virtual void save();
        virtual void defaults();

    Q_SIGNALS:
        void explicitSaved();
        void implicitSaved();

    public slots:
        void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);
        void slotCurrentChanged(const QModelIndex &current, const QModelIndex&);

    private:
        Ui::PolkitActionsMainView *m_ui;
        PolkitKde::PoliciesModel *m_model;
        PolkitKde::AuthorizationsFilterModel *m_proxyModel;
        QPointer<PolkitKde::ActionWidget> m_actionWidget;
};

#endif // POLKITACTIONSKCM_H
