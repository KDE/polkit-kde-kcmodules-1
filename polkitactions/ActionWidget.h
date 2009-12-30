/* This file is part of the KDE project

   Copyright (C) 2009 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef ACTIONWIDGET_H
#define ACTIONWIDGET_H

#include <QtGui/QWidget>

namespace PolkitQt1 {
class ActionDescription;
}

namespace Ui {
class ActionWidget;
}

namespace PolkitKde {

class ActionWidget : public QWidget
{
    public:
        explicit ActionWidget(PolkitQt1::ActionDescription *action, QWidget* parent = 0);
        virtual ~ActionWidget();

    private:
        Ui::ActionWidget *m_ui;
        PolkitQt1::ActionDescription *m_action;
};

}

#endif // ACTIONWIDGET_H
