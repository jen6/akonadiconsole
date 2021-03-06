/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef AKONADI_COLLECTIONATTRIBUTESPAGE_H
#define AKONADI_COLLECTIONATTRIBUTESPAGE_H

#include "ui_collectionattributespage.h"

#include <AkonadiWidgets/collectionpropertiespage.h>

class QStandardItem;
class QStandardItemModel;

class CollectionAttributePage : public Akonadi::CollectionPropertiesPage
{
    Q_OBJECT
public:
    explicit CollectionAttributePage(QWidget *parent = nullptr);

    void load(const Akonadi::Collection &col) override;
    void save(Akonadi::Collection &col) override;

private Q_SLOTS:
    void addAttribute();
    void delAttribute();
    void attributeChanged(QStandardItem *item);

private:
    Ui::CollectionAttributesPage ui;
    QStandardItemModel *mModel = nullptr;
    QSet<QString> mDeleted;
    QSet<QString> mChanged;
};

#endif
