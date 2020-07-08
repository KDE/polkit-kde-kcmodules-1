/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef ACTIONWIDGET_H
#define ACTIONWIDGET_H

#include <QWidget>
#include "PKLAEntry.h"
#include <PolkitQt1/ActionDescription>

class QComboBox;
class QListWidgetItem;
namespace Ui
{
class ActionWidget;
}

namespace PolkitKde
{

class ActionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActionWidget(QWidget *parent = nullptr);
    virtual ~ActionWidget();

    static int comboBoxIndexFor(PolkitQt1::ActionDescription::ImplicitAuthorization auth);
    static PolkitQt1::ActionDescription::ImplicitAuthorization implicitAuthorizationFor(int comboBoxIndex);

    PKLAEntryList entries() const;
    PKLAEntryList implicitEntries() const;
    bool isExplicitSettingsChanged() const;
    bool isImplicitSettingsChanged() const;

public Q_SLOTS:
    void setAction(const PolkitQt1::ActionDescription &action);
    void computeActionPolicies();
    void editExplicitPKLAEntry(QListWidgetItem *item);
    void addExplicitPKLAEntry();
    void implicitSettingsSaved();
    void explicitSettingsSaved();

private Q_SLOTS:
    void movePKLADown();
    void movePKLAUp();
    void explicitSelectionChanged(QListWidgetItem *current, QListWidgetItem *);
    void removePKLAEntry();
    void anyImplicitSettingChanged();
    void activeImplicitSettingChanged();
    void inactiveImplicitSettingChanged();

Q_SIGNALS:
    void changed();

private:
    void implicitSettingChanged(PolkitQt1::ActionDescription::ImplicitAuthorization auth, QComboBox *box);
    void setImplicitAuthorization(PolkitQt1::ActionDescription::ImplicitAuthorization auth, QComboBox *box);
    void addImplicitSetting();
    void addNewPKLAEntry(const PKLAEntry &entry);
    QString formatPKLAEntry(const PKLAEntry &entry);
    QString formatIdentities(const QString &identities);
    bool reloadPKLAs();

    bool m_explicitIsChanged;
    bool m_implicitIsChanged;
    bool m_PKLALoaded;
    Ui::ActionWidget *m_ui;
    PKLAEntry m_current_policy;
    PKLAEntryList m_entries;
    PKLAEntryList m_implicit_entries;
};

}

#endif // ACTIONWIDGET_H
