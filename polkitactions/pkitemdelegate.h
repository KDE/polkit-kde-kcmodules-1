/*  This file is part of the KDE project
    Copyright (C) 2008 Alessandro Diaferia <alediaferia@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef PKITEMDELEGATE_H
#define PKITEMDELEGATE_H

#include <QStyledItemDelegate>

#include <KIcon>

class QStyleOptionViewItem;

namespace PolkitKde {

class PkItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PkItemDelegate(QObject *parent = 0);
    ~PkItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

class PKLAItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    PKLAItemDelegate(QObject *parent = 0);
    ~PKLAItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    KIcon m_passwordIcon;
};

}

#endif
