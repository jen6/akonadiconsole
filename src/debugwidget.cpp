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

#include "debugwidget.h"

#include "tracernotificationinterface.h"
#include "connectionpage.h"

#include <AkonadiWidgets/controlgui.h>

#include <QTabWidget>
#include <KTextEdit>
#include <AkonadiCore/ServerManager>

#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QFileDialog>

using org::freedesktop::Akonadi::DebugInterface;

DebugWidget::DebugWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QString service = QStringLiteral("org.freedesktop.Akonadi");
    if (Akonadi::ServerManager::hasInstanceIdentifier()) {
        service += QLatin1Char('.') + Akonadi::ServerManager::instanceIdentifier();
    }
    mDebugInterface = new DebugInterface(service, QStringLiteral("/debug"), QDBusConnection::sessionBus(), this);
    QCheckBox *cb = new QCheckBox(QStringLiteral("Enable debugger"), this);
    cb->setChecked(mDebugInterface->isValid() && mDebugInterface->tracer().value() == QLatin1String("dbus"));
    connect(cb, &QCheckBox::toggled, this, &DebugWidget::enableDebugger);
    layout->addWidget(cb);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->setObjectName(QStringLiteral("debugSplitter"));
    layout->addWidget(splitter);

    mConnectionPage = new ConnectionPage(QStringLiteral("All"), splitter);
    mConnectionPage->showAllConnections(true);

    mGeneralView = new KTextEdit(splitter);
    mGeneralView->setReadOnly(true);


    org::freedesktop::Akonadi::TracerNotification *iface = new org::freedesktop::Akonadi::TracerNotification(QString(), QStringLiteral("/tracing/notifications"), QDBusConnection::sessionBus(), this);

    connect(iface, &org::freedesktop::Akonadi::TracerNotification::signalEmitted, this, &DebugWidget::signalEmitted);
    connect(iface, &org::freedesktop::Akonadi::TracerNotification::warningEmitted, this, &DebugWidget::warningEmitted);
    connect(iface, &org::freedesktop::Akonadi::TracerNotification::errorEmitted, this, &DebugWidget::errorEmitted);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    layout->addLayout(buttonLayout);

    QPushButton *clearGeneralButton = new QPushButton(QStringLiteral("Clear Information View"), this);
    QPushButton *clearFilteredButton = new QPushButton(QStringLiteral("Clear Filtered Messages"), this);
    QPushButton *clearAllButton = new QPushButton(QStringLiteral("Clear All Messages"), this);
    QPushButton *saveRichtextButton = new QPushButton(QStringLiteral("Save Filtered Messages ..."), this);
    QPushButton *saveRichtextEverythingButton = new QPushButton(QStringLiteral("Save All Messages ..."), this);

    buttonLayout->addWidget(clearFilteredButton);
    buttonLayout->addWidget(clearAllButton);
    buttonLayout->addWidget(clearGeneralButton);
    buttonLayout->addWidget(saveRichtextButton);
    buttonLayout->addWidget(saveRichtextEverythingButton);

    connect(clearFilteredButton, &QPushButton::clicked, mConnectionPage, &ConnectionPage::clearFiltered);
    connect(clearAllButton, &QPushButton::clicked, mConnectionPage, &ConnectionPage::clear);
    connect(clearGeneralButton, &QPushButton::clicked, mGeneralView, &KTextEdit::clear);
    connect(saveRichtextButton, &QPushButton::clicked, this, &DebugWidget::saveRichText);
    connect(saveRichtextEverythingButton, &QPushButton::clicked, this, &DebugWidget::saveEverythingRichText);

    Akonadi::ControlGui::widgetNeedsAkonadi(this);
}

void DebugWidget::signalEmitted(const QString &signalName, const QString &msg)
{
    mGeneralView->append(QStringLiteral("<font color=\"green\">%1 ( %2 )</font>").arg(signalName, msg));
}

void DebugWidget::warningEmitted(const QString &componentName, const QString &msg)
{
    mGeneralView->append(QStringLiteral("<font color=\"blue\">%1: %2</font>").arg(componentName, msg));
}

void DebugWidget::errorEmitted(const QString &componentName, const QString &msg)
{
    mGeneralView->append(QStringLiteral("<font color=\"red\">%1: %2</font>").arg(componentName, msg));
}

void DebugWidget::enableDebugger(bool enable)
{
    mDebugInterface->setTracer(enable ? QStringLiteral("dbus") : QStringLiteral("null"));
}

void DebugWidget::saveRichText()
{
    const QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    file.write(mConnectionPage->toHtmlFiltered().toUtf8());
    file.close();
}

void DebugWidget::saveEverythingRichText()
{
    const QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    file.write(mConnectionPage->toHtml().toUtf8());
    file.close();
}
