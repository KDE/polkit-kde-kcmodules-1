/* This file is part of the KDE project

   Copyright (C) 2010 Dario Freddi <drf@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef EXPLICITAUTHORIZATIONDIALOG_H
#define EXPLICITAUTHORIZATIONDIALOG_H

#include <KDialog>
#include "../PKLAEntry.h"

class ExplicitAuthorizationDialog : public KDialog
{
    Q_OBJECT
    public:
        explicit ExplicitAuthorizationDialog(const PKLAEntry &entry, QWidget* parent = 0);
        explicit ExplicitAuthorizationDialog(QWidget* parent = 0);
        virtual ~ExplicitAuthorizationDialog();

    private:
        PKLAEntry m_entry;
};

#endif // EXPLICITAUTHORIZATIONDIALOG_H
