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

#include "mainwindow.h"
#include "config-akonadiconsole.h"

#ifdef ENABLE_LIBKDEPIM
#include <Libkdepim/UiStateSaver>
#endif

#include <KActionCollection>
#include <KConfigGroup>
#include <KMessageBox>
#include <KStandardAction>

#include <KSharedConfig>

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    mMain = new MainWidget(this);
    setCentralWidget(mMain);

    KStandardAction::quit(qApp, &QApplication::quit, actionCollection());

    setupGUI(Keys /*| ToolBar | StatusBar*/ | Save | Create, QStringLiteral("akonadiconsoleui.rc"));
#ifdef ENABLE_LIBKDEPIM
    KPIM::UiStateSaver::restoreState(this, KConfigGroup(KSharedConfig::openConfig(), "UiState"));
#endif
    KMessageBox::information(this, QStringLiteral("<p>Akonadi Console is purely a development tool. "
                             "It allows you to view and change internal data structures of Akonadi. "
                             "You should only change data in here if you know what you are doing, otherwise "
                             "you risk damaging or losing your personal information management data.<br/>"
                             "<b>Use at your own risk!</b></p>"),
                             QString(), QStringLiteral("UseAtYourOwnRiskWarning"));
}

MainWindow::~MainWindow()
{
    delete mMain;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#ifdef ENABLE_LIBKDEPIM
    KConfigGroup config(KSharedConfig::openConfig(), "UiState");
    KPIM::UiStateSaver::saveState(this, config);
    KSharedConfig::openConfig()->sync();
#endif
    KXmlGuiWindow::closeEvent(event);
}
