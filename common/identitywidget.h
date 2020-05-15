/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef IDENTITYWIDGET_H
#define IDENTITYWIDGET_H

#include <QWidget>

#include <kdemacros.h>

namespace Ui {
    class IdentityWidget;
}

class KDE_EXPORT IdentityWidget : public QWidget
{
    Q_OBJECT
    public:
        enum IdentityType {
            UserIdentity = 0,
            GroupIdentity = 1
        };

        IdentityWidget(IdentityType type, const QString &name, QWidget* parent = 0);
        IdentityWidget(QWidget* parent = 0);
        ~IdentityWidget();
        void setIdentityType(IdentityType type);
        void setIdentityName(const QString &name);

        IdentityType identityType() const;
        QString identityName() const;

    Q_SIGNALS:
        void changed();

    private Q_SLOTS:
        void populateIdentityNameBox();

    private:
        void init(IdentityType type);

        Ui::IdentityWidget *m_ui;
};

#endif // IDENTITYWIDGET_H
