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

#include "connectionpage.h"

#include "debugfiltermodel.h"
#include "debugmodel.h"

#include <KCheckComboBox>

#include <QLabel>
#include <QHeaderView>
#include <QTableView>

#include <QVBoxLayout>
#include <QFontDatabase>

#include "tracernotificationinterface.h"
#include <QStandardItemModel>

Q_DECLARE_METATYPE(DebugModel::Message)

ConnectionPage::ConnectionPage(const QString &identifier, QWidget *parent)
    : QWidget(parent), mIdentifier(identifier), mShowAllConnections(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    auto h = new QHBoxLayout;
    layout->addLayout(h);

    h->addWidget(new QLabel(QStringLiteral("Programs:")));
    h->addWidget(mSenderFilter = new KPIM::KCheckComboBox());
    h->setStretchFactor(mSenderFilter, 2);

    mModel = new DebugModel(this);
    mModel->setSenderFilterModel(qobject_cast<QStandardItemModel*>(mSenderFilter->model()));

    mFilterModel = new DebugFilterModel(this);
    mFilterModel->setSourceModel(mModel);
    mFilterModel->setSenderFilter(mSenderFilter);

    auto mDataView = new QTableView(this);
    mDataView->setModel(mFilterModel);
    mDataView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    mDataView->horizontalHeader()->setStretchLastSection(true);
    mDataView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    layout->addWidget(mDataView);

    org::freedesktop::Akonadi::TracerNotification *iface = new org::freedesktop::Akonadi::TracerNotification(QString(), QStringLiteral("/tracing/notifications"), QDBusConnection::sessionBus(), this);

    connect(iface, &OrgFreedesktopAkonadiTracerNotificationInterface::connectionDataInput,
            this, &ConnectionPage::connectionDataInput);
    connect(iface, &OrgFreedesktopAkonadiTracerNotificationInterface::connectionDataOutput,
            this, &ConnectionPage::connectionDataOutput);
    connect(mDataView->horizontalHeader(), &QHeaderView::sectionResized,
            mDataView, &QTableView::resizeRowsToContents);
    connect(mFilterModel, &QAbstractItemModel::modelReset,
            mDataView, &QTableView::resizeRowsToContents);
    connect(mFilterModel, &QAbstractItemModel::layoutChanged,
            mDataView, &QTableView::resizeRowsToContents);
}

void ConnectionPage::connectionDataInput(const QString &identifier, const QString &msg)
{
    if (mShowAllConnections || identifier == mIdentifier) {
        mModel->addMessage(identifier, DebugModel::ClientToServer, msg);
    }
}

void ConnectionPage::connectionDataOutput(const QString &identifier, const QString &msg)
{
    if (mShowAllConnections || identifier == mIdentifier) {
        mModel->addMessage(identifier, DebugModel::ServerToClient, msg);
    }
}

void ConnectionPage::showAllConnections(bool show)
{
    mShowAllConnections = show;
}

QString ConnectionPage::toHtml(QAbstractItemModel *model) const
{
    QString ret;
    int anz = model->rowCount();
    for (int row=0; row < anz; row++) {
        const auto message = model->data(model->index(row, 0), DebugModel::MessageRole).value<DebugModel::Message>();
        const auto &sender = model->data(model->index(row, DebugModel::SenderColumn)).toString();

        ret += (message.direction==DebugModel::ClientToServer ? QStringLiteral("<- ") : QStringLiteral("-> "));
        ret += sender + QStringLiteral(" ") + message.message + QStringLiteral("\n");

    }
    return ret;
}

QString ConnectionPage::toHtmlFiltered() const
{
    return toHtml(mFilterModel);
}

QString ConnectionPage::toHtml() const
{
    return toHtml(mModel);
}

void ConnectionPage::clear()
{
    mModel->removeRows(0, mModel->rowCount());
}

void ConnectionPage::clearFiltered()
{
    mFilterModel->removeRows(0, mFilterModel->rowCount());
}
