/*
  * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef MONITORSWIDGET_H
#define MONITORSWIDGET_H

#include <QWidget>
#include <QTreeView>

class QStandardItem;
namespace Akonadi {
class TagFetchScope;
class CollectionFetchScope;
}

class MonitorsModel;

class MonitorsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorsWidget(QWidget *parent = nullptr);
    virtual ~MonitorsWidget();

private Q_SLOTS:
    void onSubscriberSelected(const QModelIndex &index);

    void populateTagFetchScope(QStandardItem *parent,
                               const Akonadi::TagFetchScope &tfs);
    void populateCollectionFetchScope(QStandardItem *parent,
                                      const Akonadi::CollectionFetchScope &cfs);
private:
    QTreeView *mTreeView = nullptr;
    QTreeView *mSubscriberView = nullptr;
    MonitorsModel *mModel = nullptr;
};

#endif // MONITORSWIDGET_H
