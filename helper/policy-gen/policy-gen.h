/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Dario Freddi <drf@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#ifndef POLICY_GEN_H
#define POLICY_GEN_H

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QHash>

struct Action {
    QString name;

    QHash<QString, QString> descriptions;
    QHash<QString, QString> messages;

    QString policy;
    QString persistence;
};

extern void output(QList<Action> actions, QHash<QString, QString> domain);


#endif
