/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef POLKITACTIONSKCM_H
#define POLKITACTIONSKCM_H

#include <kcmodule.h>
#include <PolkitQt1/Authority>

namespace PolkitKde {
class PoliciesModel;
class AuthorizationsFilterModel;
}

namespace Ui {
    class PolkitActionsMainView;
}

class PolkitActionsKCM : public KCModule
{
    Q_OBJECT
    public:
        PolkitActionsKCM(QWidget* parent = 0, const QVariantList& args = QVariantList());
        virtual ~PolkitActionsKCM();

        virtual void load();
        virtual void save();
        virtual void defaults();

    public slots:
        void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);

    private:
        Ui::PolkitActionsMainView *m_ui;
        PolkitKde::PoliciesModel *m_model;
        PolkitKde::AuthorizationsFilterModel *m_proxyModel;
};

#endif // POLKITACTIONSKCM_H
