/* This file is part of the KDE project

   Copyright (C) 2009-2010 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "ActionWidget.h"

#include "ui_actionwidget.h"

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <qdbuspendingcall.h>
#include <QtDBus/qdbusmetatype.h>
#include <PolkitQt1/ActionDescription>
#include <QDebug>
#include <KMessageBox>
#include <KLocalizedString>
#include "pkitemdelegate.h"
#include "explicitauthorizationdialog.h"
#include <QSettings>
#include <QPointer>

namespace PolkitKde
{

bool orderByPriorityLessThan(const PKLAEntry &e1, const PKLAEntry &e2)
{
    return e1.fileOrder < e2.fileOrder;
}

ActionWidget::ActionWidget(QWidget *parent)
    : QWidget(parent)
    , m_PKLALoaded(false)
    , m_ui(new Ui::ActionWidget)
{
    m_ui->setupUi(this);

    // Icons 'n stuff
    m_ui->removeButton->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    m_ui->addLocalButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    m_ui->moveDownButton->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    m_ui->moveUpButton->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));

    m_ui->localAuthListWidget->setItemDelegate(new PKLAItemDelegate);
    this->setEnabled(false);

    connect(m_ui->localAuthListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(editExplicitPKLAEntry(QListWidgetItem*)));
    connect(m_ui->localAuthListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(explicitSelectionChanged(QListWidgetItem*,QListWidgetItem*)));
    connect(m_ui->addLocalButton, SIGNAL(clicked(bool)),
            this, SLOT(addExplicitPKLAEntry()));
    connect(m_ui->removeButton, SIGNAL(clicked(bool)),
            this, SLOT(removePKLAEntry()));
    connect(m_ui->moveDownButton, SIGNAL(clicked(bool)),
            this, SLOT(movePKLADown()));
    connect(m_ui->moveUpButton, SIGNAL(clicked(bool)),
            this, SLOT(movePKLAUp()));
    connect(m_ui->anyComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(anyImplicitSettingChanged()));
    connect(m_ui->inactiveComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(inactiveImplicitSettingChanged()));
    connect(m_ui->activeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(activeImplicitSettingChanged()));
}

ActionWidget::~ActionWidget()
{
    delete m_ui;
}

bool ActionWidget::reloadPKLAs()
{
    m_entries.clear();
    m_implicit_entries.clear();
    m_explicitIsChanged = false;
    m_implicitIsChanged = false;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.polkitkde1.helper"),
                                                          QStringLiteral("/Helper"),
                                                          QStringLiteral("org.kde.polkitkde1.helper"),
                                                          QStringLiteral("retrievePolicies"));
    QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message);
    reply.waitForFinished();
    if (reply.isError()) {
        const QDBusError dbusError = reply.error();
        // from PolkitKde1Helper::retrievePolicies()
        const QString requiredAuth(QStringLiteral("org.kde.polkit1.readauthorizations"));

        KMessageBox::detailedError(this,
                                   i18n("<qt>Could not retrieve PolicyKit policies.<br>Either the authorization failed, or there is a system configuration problem."),
                                   i18n("<qt>Bus: system<br>Call: %1 %2 %3.%4<br>Error: %5 \"%6\"<br>Action: %7",
                                        message.service(), message.path(), message.interface(), message.member(),
                                        dbusError.name(), dbusError.message(),
                                        requiredAuth));
        return (false);
    }

    const QDBusMessage r = reply.reply();
    if (r.arguments().count() >= 1) {
        QVariantList vlist;
        r.arguments().first().value<QDBusArgument>() >> vlist;
        foreach (const QVariant &variant, vlist) {
            PKLAEntry entry;
            variant.value<QDBusArgument>() >> entry;
            qDebug() << entry.title;
            m_entries.append(entry);
        }
    }

    if (!m_current_policy.action.isEmpty()) {
        computeActionPolicies();
    }

    return (true);
}

void ActionWidget::computeActionPolicies()
{
    qDebug();
    m_ui->localAuthListWidget->clear();
    std::sort(m_entries.begin(), m_entries.end(), orderByPriorityLessThan);
    foreach (const PKLAEntry &entry, m_entries) {
        QStringList realActions = entry.action.split(QLatin1Char(';'));
        qDebug() << entry.action << m_current_policy.action;
        if (realActions.contains(m_current_policy.action)) {
            // Match! Is it, actually, an implicit override?
            /* if (entry.title == "PolkitKdeOverrideImplicit") {
                // It is!
                qDebug() << "Found implicit override";
                setImplicitAuthorization(PKLAEntry::implFromText(entry.resultActive), m_ui->activeComboBox);
                setImplicitAuthorization(PKLAEntry::implFromText(entry.resultInactive), m_ui->inactiveComboBox);
                setImplicitAuthorization(PKLAEntry::implFromText(entry.resultAny), m_ui->anyComboBox);
            } else { */
                // TODO: Add it to the local auths
                //LocalAuthorization *auth = new LocalAuthorization(entry);
                qDebug() << "Found PKLA override";
                QListWidgetItem *item = new QListWidgetItem(entry.title);
                item->setData(Qt::UserRole, formatPKLAEntry(entry));
                m_ui->localAuthListWidget->addItem(item);
            // }
        }
    }

    // Trigger the selection
    if (!m_ui->localAuthListWidget->selectedItems().isEmpty()) {
        explicitSelectionChanged(m_ui->localAuthListWidget->selectedItems().first(), 0);
    } else {
        explicitSelectionChanged(0, 0);
    }
}

QString ActionWidget::formatPKLAEntry(const PKLAEntry &entry)
{
    QString authorizationText;

    if (entry.resultActive != m_current_policy.resultActive) {
        authorizationText.append(i18n("'%1' on active console", entry.resultActive));
        authorizationText.append(QStringLiteral(", "));
    }
    if (entry.resultInactive != m_current_policy.resultInactive) {
        authorizationText.append(i18n("'%1' on inactive console", entry.resultActive));
        authorizationText.append(QStringLiteral(", "));
    }
    if (entry.resultAny != m_current_policy.resultAny) {
        authorizationText.append(i18n("'%1' on any console", entry.resultActive));
        authorizationText.append(QStringLiteral(", "));
    }

    if (authorizationText.endsWith(QLatin1String(", "))) {
        authorizationText.remove(-1, 2);
    }

    return i18np("%2 has the following policy: %3", "%2 have the following policy: %3",
                 entry.identity.split(QLatin1Char(';')).count(), formatIdentities(entry.identity), authorizationText);
}

QString ActionWidget::formatIdentities(const QString &identities)
{
    QString rettext;
    QStringList realIdentities = identities.split(QLatin1Char(';'));

    foreach (const QString &identity, realIdentities) {
        if (identity.startsWith(QLatin1String("unix-user:"))) {
            rettext.append(identity.split(QStringLiteral("unix-user:")).last());
            rettext.append(QStringLiteral(", "));
        }
        if (identity.startsWith(QLatin1String("unix-group:"))) {
            rettext.append(i18n("%1 group", identity.split(QStringLiteral("unix-group:")).last()));
            rettext.append(QStringLiteral(", "));
        }
    }

    if (rettext.endsWith(QLatin1String(", "))) {
        rettext = rettext.remove(rettext.length() - 2, 2);
    }

    return rettext;
}

void ActionWidget::setImplicitAuthorization(PolkitQt1::ActionDescription::ImplicitAuthorization auth, QComboBox *box)
{
    box->setCurrentIndex(comboBoxIndexFor(auth));
}

int ActionWidget::comboBoxIndexFor(PolkitQt1::ActionDescription::ImplicitAuthorization auth)
{
    switch (auth) {
    case PolkitQt1::ActionDescription::Authorized:
        return 0;
    case PolkitQt1::ActionDescription::NotAuthorized:
        return 1;
    case PolkitQt1::ActionDescription::AuthenticationRequired:
        return 4;
    case PolkitQt1::ActionDescription::AuthenticationRequiredRetained:
        return 5;
    case PolkitQt1::ActionDescription::AdministratorAuthenticationRequired:
        return 2;
    case PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained:
        return 3;
    default:
        break;
    }

    return 1;
}

PolkitQt1::ActionDescription::ImplicitAuthorization ActionWidget::implicitAuthorizationFor(int comboBoxIndex)
{
    switch (comboBoxIndex) {
    case 0:
        return PolkitQt1::ActionDescription::Authorized;
    case 1:
        return PolkitQt1::ActionDescription::NotAuthorized;
    case 4:
        return PolkitQt1::ActionDescription::AuthenticationRequired;
    case 5:
        return PolkitQt1::ActionDescription::AuthenticationRequiredRetained;
    case 2:
        return PolkitQt1::ActionDescription::AdministratorAuthenticationRequired;
    case 3:
        return PolkitQt1::ActionDescription::AdministratorAuthenticationRequiredRetained;
    default:
        break;
    }

    return PolkitQt1::ActionDescription::Unknown;
}

void ActionWidget::setAction(const PolkitQt1::ActionDescription &action)
{
    bool implicit_override = false;

    // Check if PKLA's are loaded
    if (!m_PKLALoaded) {
        if (!reloadPKLAs()) {
            setEnabled(false);
            return;
        }
        m_PKLALoaded = true;
    }
    // Check for implicit override
    foreach (const PKLAEntry &entry, m_implicit_entries) {
        if (entry.action == action.actionId()) {
            qDebug() << "Found implicit override!";
            m_current_policy = entry;
            implicit_override = true;
            break;
        }
    }

    // No implicit override found. Lets use the action
    if (!implicit_override) {
        m_current_policy.action = action.actionId();
        m_current_policy.resultActive = PKLAEntry::textFromImpl(action.implicitActive());
        m_current_policy.resultInactive = PKLAEntry::textFromImpl(action.implicitInactive());
        m_current_policy.resultAny = PKLAEntry::textFromImpl(action.implicitAny());
    }

    setImplicitAuthorization(PKLAEntry::implFromText(m_current_policy.resultActive), m_ui->activeComboBox);
    setImplicitAuthorization(PKLAEntry::implFromText(m_current_policy.resultInactive), m_ui->inactiveComboBox);
    setImplicitAuthorization(PKLAEntry::implFromText(m_current_policy.resultAny), m_ui->anyComboBox);

    m_ui->descriptionLabel->setText(action.description());
    m_ui->vendorLabel->setText(action.vendorName());
    m_ui->vendorLabel->setUrl(action.vendorUrl());
    m_ui->pixmapLabel->setPixmap(QIcon::fromTheme(action.iconName()).pixmap(64));

    computeActionPolicies();
    this->setEnabled(true);
}

void ActionWidget::editExplicitPKLAEntry(QListWidgetItem *item)
{
    foreach (const PKLAEntry &entry, m_entries) {
        if (entry.title == item->text()) {
            QPointer<ExplicitAuthorizationDialog> dialog = new ExplicitAuthorizationDialog(entry, this);
            if (dialog.data()->exec() == QDialog::Accepted) {
                dialog.data()->commitChangesToPKLA();
                PKLAEntry result = dialog.data()->pkla();
                // Register the entry. But first remove the previous one to avoid duplicates
                for (PKLAEntryList::iterator it = m_entries.begin(); it != m_entries.end(); ++it) {
                    if ((*it).title == result.title) {
                        // Erase the old one
                        m_entries.erase(it);
                        break;
                    }
                }
                addNewPKLAEntry(result);
            }

            if (dialog) {
                dialog.data()->deleteLater();
            }
        }
    }
}

void ActionWidget::addExplicitPKLAEntry()
{
    QPointer<ExplicitAuthorizationDialog> dialog = new ExplicitAuthorizationDialog(m_current_policy.action, this);
    if (dialog.data()->exec() == QDialog::Accepted) {
        dialog.data()->commitChangesToPKLA();
        PKLAEntry result = dialog.data()->pkla();
        // Register the entry.
        addNewPKLAEntry(result);
    }

    if (dialog) {
        dialog.data()->deleteLater();
    }
}

void ActionWidget::addNewPKLAEntry(const PKLAEntry &entry)
{
    PKLAEntry toInsert(entry);
    // Match it to the current config value
    QSettings settings(QStringLiteral("/etc/polkit-1/polkit-kde-1.conf"), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("General"));
    toInsert.filePriority = settings.value(QStringLiteral("PoliciesPriority"), 75).toInt();

    // If there's no file order, append it to the end of the current entries
    if (toInsert.fileOrder < 0) {
        int max = 0;
        foreach (const PKLAEntry &entry, m_entries) {
            if (entry.fileOrder > max) {
                max = entry.fileOrder;
            }
        }
        ++max;
        toInsert.fileOrder = max;
    }

    // Ok, now append it to the list
    qDebug() << "Explicit settings changed";
    m_explicitIsChanged = true;
    m_entries.append(toInsert);
    qDebug() << "Inserting entry named " << toInsert.title << " for " << toInsert.action;

    emit changed();

    // And reload the policies
    computeActionPolicies();
}

void ActionWidget::removePKLAEntry()
{
    if (m_ui->localAuthListWidget->selectedItems().isEmpty()) {
        return;
    }

    QListWidgetItem *item = m_ui->localAuthListWidget->selectedItems().first();

    // Find and erase the PKLA
    for (PKLAEntryList::iterator it = m_entries.begin(); it != m_entries.end(); ++it) {
        if ((*it).title == item->text()) {
            // Remove it
            it = m_entries.erase(it);
            break;
        }
    }
    qDebug() << "Explicit settings changed";
    m_explicitIsChanged = true;
    emit changed();

    // Reload
    computeActionPolicies();
}

void ActionWidget::explicitSelectionChanged(QListWidgetItem *current, QListWidgetItem *)
{
    if (current) {
        m_ui->removeButton->setEnabled(true);

        // Can we move up?
        m_ui->moveUpButton->setEnabled(m_ui->localAuthListWidget->currentRow() > 0);
        // Can we move down?
        m_ui->moveDownButton->setEnabled(m_ui->localAuthListWidget->currentRow() < (m_ui->localAuthListWidget->count() - 1));
    } else {
        m_ui->removeButton->setEnabled(false);
        m_ui->moveDownButton->setEnabled(false);
        m_ui->moveUpButton->setEnabled(false);
    }
}

void ActionWidget::movePKLADown()
{
    if (m_ui->localAuthListWidget->selectedItems().isEmpty()) {
        return;
    }

    QListWidgetItem *item = m_ui->localAuthListWidget->selectedItems().first();

    // Find the PKLA and change it
    PKLAEntry entry;
    for (PKLAEntryList::iterator it = m_entries.begin(); it != m_entries.end(); ++it) {
        if ((*it).title == item->text()) {
            // Decrease the priority
            ++(*it).fileOrder;
            qDebug() << (*it).title << " is now " << (*it).fileOrder;
            // Increase the priority of the next one
            ++it;
            --(*it).fileOrder;
            break;
        }
    }
    qDebug() << "Explicit settings changed";
    m_explicitIsChanged = true;
    emit changed();

    // Reload
    computeActionPolicies();
}

void ActionWidget::movePKLAUp()
{
    if (m_ui->localAuthListWidget->selectedItems().isEmpty()) {
        return;
    }

    QListWidgetItem *item = m_ui->localAuthListWidget->selectedItems().first();

    // Find the PKLA and change it
    PKLAEntry entry;
    for (PKLAEntryList::iterator it = m_entries.begin(); it != m_entries.end(); ++it) {
        if ((*it).title == item->text()) {
            // Increase the priority
            --(*it).fileOrder;
            qDebug() << (*it).title << " is now " << (*it).fileOrder;
            // Decrease the priority of the previous one
            --it;
            ++(*it).fileOrder;
            break;
        }
    }
    qDebug() << "Explicit settings changed";
    m_explicitIsChanged = true;
    emit changed();

    // Reload
    computeActionPolicies();
}

PKLAEntryList ActionWidget::entries() const
{
    return m_entries;
}

PKLAEntryList ActionWidget::implicitEntries() const
{
    return m_implicit_entries;
}

void ActionWidget::anyImplicitSettingChanged()
{
    implicitSettingChanged(PKLAEntry::implFromText(m_current_policy.resultAny), m_ui->anyComboBox);
}

void ActionWidget::activeImplicitSettingChanged()
{
    implicitSettingChanged(PKLAEntry::implFromText(m_current_policy.resultActive), m_ui->activeComboBox);
}

void ActionWidget::inactiveImplicitSettingChanged()
{
    implicitSettingChanged(PKLAEntry::implFromText(m_current_policy.resultInactive), m_ui->inactiveComboBox);
}

void ActionWidget::implicitSettingChanged(PolkitQt1::ActionDescription::ImplicitAuthorization auth, QComboBox *box)
{
    // Check if the setting has been changed or if it is just an action change
    if (auth != implicitAuthorizationFor(box->currentIndex())) {
        // The setting has been changed. Now add the new setting to the implicitlist
        addImplicitSetting();

        // Settings changed. Enable apply button.
        emit changed();
    }
}

void ActionWidget::addImplicitSetting()
{
    PKLAEntry entry;
    entry.resultAny = PKLAEntry::textFromImpl(implicitAuthorizationFor(m_ui->anyComboBox->currentIndex()));
    entry.resultActive = PKLAEntry::textFromImpl(implicitAuthorizationFor(m_ui->activeComboBox->currentIndex()));
    entry.resultInactive = PKLAEntry::textFromImpl(implicitAuthorizationFor(m_ui->inactiveComboBox->currentIndex()));
    entry.action = m_current_policy.action;

    // Before adding the new setting, delete all former settings on our list
    for (PKLAEntryList::iterator it = m_implicit_entries.begin(); it != m_implicit_entries.end(); ++it) {
        // Match! Delete old entry
        if ((*it).action == m_current_policy.action) {
            m_implicit_entries.erase(it);
            break;
        }
    }
    qDebug() << "Implicit settings changed";
    m_implicitIsChanged = true;
    m_implicit_entries.push_back(entry);

    // Update the old settings container
    m_current_policy.resultActive = entry.resultActive;
    m_current_policy.resultAny = entry.resultAny;
    m_current_policy.resultInactive = entry.resultInactive;
}

void ActionWidget::implicitSettingsSaved()
{
    m_implicitIsChanged = false;
}

void ActionWidget::explicitSettingsSaved()
{
    m_explicitIsChanged = false;
}

bool ActionWidget::isExplicitSettingsChanged() const
{
    return m_explicitIsChanged;
}

bool ActionWidget::isImplicitSettingsChanged() const
{
    return m_implicitIsChanged;
}

}
