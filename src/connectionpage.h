/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef AKONADICONSOLE_CONNECTIONPAGE_H
#define AKONADICONSOLE_CONNECTIONPAGE_H

#include <QWidget>

class DebugModel;
class DebugFilterModel;
class QAbstractItemModel;
class QTableView;

namespace KPIM {
class KCheckComboBox;
}

class ConnectionPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionPage(const QString &identifier, QWidget *parent = nullptr);

    void showAllConnections(bool);

    QString toHtml() const;
    QString toHtmlFiltered() const;

public Q_SLOTS:
    void clear();
    void clearFiltered();

private Q_SLOTS:
    void connectionDataInput(const QString &, const QString &);
    void connectionDataOutput(const QString &, const QString &);

private:
    QString toHtml(QAbstractItemModel *model) const;

    DebugModel *mModel = nullptr;
    DebugFilterModel *mFilterModel = nullptr;
    QTableView *mDataView = nullptr;
    KPIM::KCheckComboBox *mSenderFilter = nullptr;
    QString mIdentifier;
    bool mShowAllConnections;
};

#endif
