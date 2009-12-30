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

#include "PolkitActionsKCM.h"

#include <KPluginFactory>
#include <KAboutData>
#include <klocalizedstring.h>
#include <PolkitQt1/Authority>
#include "ui_mainview.h"
#include "PoliciesModel.h"
#include "AuthorizationsFilterModel.h"
#include "pkitemdelegate.h"

K_PLUGIN_FACTORY(KCMPolkitActionsFactory,
                 registerPlugin<PolkitActionsKCM>();
                )
K_EXPORT_PLUGIN(KCMPolkitActionsFactory("kcm_polkitactions"))

PolkitActionsKCM::PolkitActionsKCM(QWidget* parent, const QVariantList& args)
    : KCModule(KCMPolkitActionsFactory::componentData(), parent, args)
    , m_ui(new Ui::PolkitActionsMainView)
{
    KAboutData *about =
        new KAboutData("kcm_polkitactions", "kcm_polkitactions", ki18n("Global system policy settings"),
                       "1.0.0", ki18n("A configuration for polkit-1 system administrators and policy priorities"),
                       KAboutData::License_GPL, ki18n("(c), 2009 Dario Freddi"),
                       ki18n("From this module, you can configure system administrators and priorities "
                             "for the policies defined in the Actions module"));

    about->addAuthor(ki18n("Dario Freddi"), ki18n("Maintainer") , "drf@kde.org", "http://drfav.wordpress.com");

    setAboutData(about);

    // Build the UI
    m_ui->setupUi(this);
    m_model = new PolkitKde::PoliciesModel(this);
    m_proxyModel = new PolkitKde::AuthorizationsFilterModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_ui->treeView->setModel(m_proxyModel);
    m_ui->treeView->setItemDelegate(new PolkitKde::PkItemDelegate(this));

    connect(m_ui->searchLine, SIGNAL(textChanged(QString)),
            m_proxyModel, SLOT(setFilterRegExp(QString)));

    // Initialize
    connect(PolkitQt1::Authority::instance(), SIGNAL(checkAuthorizationFinished(PolkitQt1::Authority::Result)),
            this, SLOT(slotCheckAuthorizationFinished(PolkitQt1::Authority::Result)));
    connect(PolkitQt1::Authority::instance(), SIGNAL(enumerateActionsFinished(PolkitQt1::ActionDescription::List)),
            m_model, SLOT(setCurrentEntries(PolkitQt1::ActionDescription::List)));
    PolkitQt1::Authority::instance()->enumerateActions();
}

PolkitActionsKCM::~PolkitActionsKCM()
{

}

void PolkitActionsKCM::load()
{
KCModule::load();
}

void PolkitActionsKCM::save()
{
KCModule::save();
}

void PolkitActionsKCM::defaults()
{
KCModule::defaults();
}

void PolkitActionsKCM::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{

}

