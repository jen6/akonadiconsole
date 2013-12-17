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

#include "agentwidget.h"
#include "agentconfigdialog.h"

#include <akonadi/agenttypedialog.h>
#include <akonadi/agentinstancewidget.h>
#include <akonadi/agentmanager.h>
#include <Akonadi/AgentFilterProxyModel>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/control.h>
#include <akonadi/private/notificationmessage_p.h>

#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardDirs>
#include <KStandardGuiItem>
#include <KTextEdit>
#include <KLineEdit>

#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QMetaObject>
#include <QMetaMethod>
#include <QResizeEvent>

class TextDialog : public KDialog
{
  public:
    TextDialog( QWidget *parent = 0 )
      : KDialog( parent )
    {
      setButtons( Ok );

      mText = new KTextEdit;
      mText->setReadOnly(true);
      setMainWidget( mText );
      setInitialSize( QSize( 400, 600 ) );
    }

    void setText( const QString &text )
    {
      mText->setPlainText( text );
    }

  private:
    KTextEdit *mText;
};

using namespace Akonadi;

AgentWidget::AgentWidget( QWidget *parent )
  : QWidget( parent )
{
  ui.setupUi( this );

  connect( ui.instanceWidget, SIGNAL(doubleClicked(Akonadi::AgentInstance)), SLOT(configureAgent()) );
  connect( ui.instanceWidget, SIGNAL(currentChanged(Akonadi::AgentInstance,Akonadi::AgentInstance)), SLOT(currentChanged()) );
  connect( ui.instanceWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)) );

  connect( ui.instanceWidget->view()->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged()) );
  connect( ui.instanceWidget->view()->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(selectionChanged()) );
  connect( ui.instanceWidget->view()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(slotDataChanged(QModelIndex,QModelIndex)) );

  currentChanged();

  ui.addButton->setGuiItem( KStandardGuiItem::add() );
  connect( ui.addButton, SIGNAL(clicked()), this, SLOT(addAgent()) );

  ui.removeButton->setGuiItem( KStandardGuiItem::remove() );
  connect( ui.removeButton, SIGNAL(clicked()), this, SLOT(removeAgent()) );

  mConfigMenu = new QMenu( i18n("Configure"), this );
  mConfigMenu->addAction( i18n("Configure Natively..."), this, SLOT(configureAgent()) );
  mConfigMenu->addAction( i18n("Configure Remotely..."), this, SLOT(configureAgentRemote()) );
  mConfigMenu->setIcon( KStandardGuiItem::configure().icon() );
  ui.configButton->setGuiItem( KStandardGuiItem::configure() );
  ui.configButton->setMenu( mConfigMenu );
  connect( ui.configButton, SIGNAL(clicked()), this, SLOT(configureAgent()) );

  mSyncMenu = new QMenu( i18n("Synchronize"), this );
  mSyncMenu->addAction( i18n("Synchronize All"), this, SLOT(synchronizeAgent()) );
  mSyncMenu->addAction( i18n("Synchronize Collection Tree"), this, SLOT(synchronizeTree()) );
  mSyncMenu->setIcon( KIcon("view-refresh" ) );
  ui.syncButton->setMenu( mSyncMenu );
  ui.syncButton->setIcon( KIcon( "view-refresh" ) );
  connect( ui.syncButton, SIGNAL(clicked()), this, SLOT(synchronizeAgent()) );

  ui.abortButton->setIcon( KIcon("dialog-cancel") );
  connect( ui.abortButton, SIGNAL(clicked()), this, SLOT(abortAgent()) );
  ui.restartButton->setIcon( KIcon( "system-reboot" ) ); //FIXME: Is using system-reboot icon here a good idea?
  connect( ui.restartButton, SIGNAL(clicked()), SLOT(restartAgent()) );

  ui.mFilterAccount->setProxy( ui.instanceWidget->agentFilterProxyModel() );
  ui.mFilterAccount->lineEdit()->setTrapReturnKey( true );

  Control::widgetNeedsAkonadi( this );
}

void AgentWidget::addAgent()
{
  QPointer<Akonadi::AgentTypeDialog> dlg = new Akonadi::AgentTypeDialog( this );
  if ( dlg->exec() ) {
    const AgentType agentType = dlg->agentType();

    if ( agentType.isValid() ) {
      AgentInstanceCreateJob *job = new AgentInstanceCreateJob( agentType, this );
      job->configure( this );
      job->start(); // TODO: check result
    }
  }
  delete dlg;
}

void AgentWidget::selectionChanged()
{
  const bool multiSelection = ui.instanceWidget->view()->selectionModel()->selectedRows().size() > 1;
  // Only agent removal, sync and restart is possible when multiple items are selected.
  ui.configButton->setDisabled( multiSelection );

  // Restarting an agent is not possible if it's in Running status... (see AgentProcessInstance::restartWhenIdle)
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  ui.restartButton->setEnabled( agent.isValid() && agent.status() != 1 );
}

void AgentWidget::slotDataChanged( const QModelIndex& topLeft, const QModelIndex& /*bottomRight*/ )
{
  QList<QModelIndex> selectedRows = ui.instanceWidget->view()->selectionModel()->selectedRows();
  if ( selectedRows.isEmpty() ) {
    selectedRows.append( ui.instanceWidget->view()->selectionModel()->currentIndex() );
  }
  QList<int> rows;
  Q_FOREACH( const QModelIndex& index, selectedRows ) {
    rows.append( index.row() );
  }
  qSort( rows );
  // Assume topLeft.row == bottomRight.row
  if ( topLeft.row() >= rows.first() && topLeft.row() <= rows.last() ) {
    selectionChanged(); // depends on status
    currentChanged();
  }
}

void AgentWidget::removeAgent()
{
  QList<AgentInstance> list = ui.instanceWidget->selectedAgentInstances();
  if ( !list.isEmpty() )
  {
    if ( KMessageBox::questionYesNo( this,
                                     i18np( "Do you really want to delete the selected agent instance?",
                                            "Do you really want to delete these %1 agent instances?",
                                            list.size() ),
                                     list.size() == 1 ? i18n( "Agent Deletion" ) : i18n( "Multiple Agent Deletion" ),
                                     KStandardGuiItem::del(),
                                     KStandardGuiItem::cancel(),
                                     QString(),
                                     KMessageBox::Dangerous )
      == KMessageBox::Yes )
    {
      foreach( const AgentInstance &agent, list )
        AgentManager::self()->removeInstance( agent );
    }
  }
}

void AgentWidget::configureAgent()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.configure( this );
}

void AgentWidget::configureAgentRemote()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() ) {
    QPointer<AgentConfigDialog> dlg = new AgentConfigDialog( this );
    dlg->setAgentInstance( agent );
    dlg->exec();
    delete dlg;
  }
}

void AgentWidget::synchronizeAgent()
{
  QList<AgentInstance> list = ui.instanceWidget->selectedAgentInstances();
  if ( !list.isEmpty() )
    foreach( AgentInstance agent, list )
      agent.synchronize();
}

void AgentWidget::toggleOnline()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.setIsOnline( !agent.isOnline() );
}

void AgentWidget::showTaskList()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( !agent.isValid() )
    return;

  QDBusInterface iface( QString::fromLatin1(  "org.freedesktop.Akonadi.Agent.%1" ).arg( agent.identifier() ),
                        "/Debug", QString() );

  QDBusReply<QString> reply = iface.call("dumpToString");
  QString txt;
  if ( reply.isValid() )
    txt = reply.value();
  else {
    txt = reply.error().message();
  }

  QPointer<TextDialog> dlg = new TextDialog( this );
  dlg->setCaption( QLatin1String( "Resource Task List" ) );
  dlg->setText( txt );
  dlg->exec();
  delete dlg;
}

void AgentWidget::showChangeNotifications()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( !agent.isValid() )
    return;

  QDBusInterface iface( QString::fromLatin1( "org.freedesktop.Akonadi.Agent.%1" ).arg( agent.identifier() ),
                        "/Debug", QString() );

  QDBusReply<QString> reply = iface.call("dumpNotificationListToString");
  QString txt;
  if ( reply.isValid() )
    txt = reply.value();
  else {
    txt = reply.error().message();
  }

  QPointer<TextDialog> dlg = new TextDialog( this );
  dlg->setCaption( QLatin1String( "Change Notification Log" ) );
  dlg->setText( txt );

  dlg->exec();
  delete dlg;
}

void AgentWidget::synchronizeTree()
{
  QList<AgentInstance> list = ui.instanceWidget->selectedAgentInstances();
  if ( !list.isEmpty() )
    foreach( AgentInstance agent, list )
      agent.synchronizeCollectionTree();
}

void AgentWidget::abortAgent()
{
  QList<AgentInstance> list = ui.instanceWidget->selectedAgentInstances();
  if ( !list.isEmpty() )
    foreach( AgentInstance agent, list )
      agent.abortCurrentTask();
}

void AgentWidget::restartAgent()
{
  AgentInstance agent = ui.instanceWidget->currentAgentInstance();
  if ( agent.isValid() )
    agent.restart();
}

void AgentWidget::cloneAgent()
{
  mCloneSource = ui.instanceWidget->currentAgentInstance();
  if ( !mCloneSource.isValid() )
    return;
  const AgentType agentType = mCloneSource.type();
  if ( agentType.isValid() ) {
    AgentInstanceCreateJob *job = new AgentInstanceCreateJob( agentType, this );
    connect( job, SIGNAL(result(KJob*)), SLOT(cloneAgent(KJob*)) );
    job->start();
  } else {
    kWarning() << "WTF?";
  }
}

void AgentWidget::cloneAgent( KJob* job )
{
  if ( job->error() ) {
    KMessageBox::error( this, i18n("Cloneing agent failed: %1.", job->errorText() ) );
    return;
  }

  AgentInstance cloneTarget = static_cast<AgentInstanceCreateJob*>( job )->instance();
  Q_ASSERT( cloneTarget.isValid() );
  Q_ASSERT( mCloneSource.isValid() );

  QDBusInterface sourceIface( QString::fromLatin1("org.freedesktop.Akonadi.Agent.%1").arg( mCloneSource.identifier() ),
                              "/Settings" );
  if ( !sourceIface.isValid() ) {
    kError() << "Unable to obtain KConfigXT D-Bus interface of source agent" << mCloneSource.identifier();
    return;
  }

  QDBusInterface targetIface( QString::fromLatin1("org.freedesktop.Akonadi.Agent.%1").arg( cloneTarget.identifier() ),
                              "/Settings" );
  if ( !targetIface.isValid() ) {
    kError() << "Unable to obtain KConfigXT D-Bus interface of target agent" << cloneTarget.identifier();
    return;
  }

  cloneTarget.setName( mCloneSource.name() + " (Clone)" );

  // iterate over all getter methods in the source interface and call the
  // corresponding setter in the target interface
  for ( int i = 0; i < sourceIface.metaObject()->methodCount(); ++i ) {
    const QMetaMethod method = sourceIface.metaObject()->method( i );
    if ( QByteArray( method.typeName() ).isEmpty() ) // returns void
      continue;
    const QByteArray signature( method.signature() );
    if ( signature.isEmpty() )
      continue;
    if ( signature.startsWith( "set" ) || !signature.contains( "()" ) ) // setter or takes parameters // krazy:exclude=strings
      continue;
    if ( signature.startsWith( "Introspect" ) ) // D-Bus stuff // krazy:exclude=strings
      continue;
    const QString methodName = QString::fromLatin1( signature.left( signature.indexOf( '(' ) ) );
    const QDBusMessage reply = sourceIface.call( methodName );
    if ( !reply.arguments().count() == 1 ) {
      kError() << "call to method" << signature << "failed: " << reply.arguments() << reply.errorMessage();
      continue;
    }
    const QString setterName = QLatin1String("set") + methodName.at( 0 ).toUpper() + methodName.mid( 1 );
    targetIface.call( setterName, reply.arguments().at( 0 ) );
  }

  cloneTarget.reconfigure();
}

void AgentWidget::currentChanged()
{
  AgentInstance instance = ui.instanceWidget->currentAgentInstance();
  ui.removeButton->setEnabled( instance.isValid() );
  ui.configButton->setEnabled( instance.isValid() );
  ui.syncButton->setEnabled( instance.isValid() );
  ui.restartButton->setEnabled( instance.isValid() );

  if ( instance.isValid() ) {
    ui.identifierLabel->setText( instance.identifier() );
    ui.typeLabel->setText( instance.type().name() );
    QString onlineStatus = ( instance.isOnline() ? i18n( "Online" ) : i18n( "Offline" ) );
    QString agentStatus;
    switch( instance.status() ) {
    case AgentInstance::Idle: agentStatus =
        i18nc( "agent is in an idle state", "Idle" );
      break;
    case AgentInstance::Running: agentStatus =
        i18nc( "agent is running", "Running (%1%)", instance.progress() );
      break;
    case AgentInstance::Broken: agentStatus =
        i18nc( "agent is broken somehow", "Broken" );
      break;
    case AgentInstance::NotConfigured: agentStatus =
        i18nc( "agent is not yet configured", "Not Configured" );
      break;
    }
    ui.statusLabel->setText(
      i18nc( "Two statuses, for example \"Online, Running (66%)\" or \"Offline, Broken\"",
             "%1, %2", onlineStatus, agentStatus ) );
    ui.statusMessageLabel->setText( instance.statusMessage() );
    ui.capabilitiesLabel->setText( instance.type().capabilities().join( ", " ) );
    ui.mimeTypeLabel->setText( instance.type().mimeTypes().join( ", " ) );
  } else {
    ui.identifierLabel->setText( QString() );
    ui.typeLabel->setText( QString() );
    ui.statusLabel->setText( QString() );
    ui.capabilitiesLabel->setText( QString() );
    ui.mimeTypeLabel->setText( QString() );
  }
}

void AgentWidget::showContextMenu(const QPoint& pos)
{
  QMenu menu( this );
  menu.addAction( KIcon("list-add"), i18n("Add Agent..."), this, SLOT(addAgent()) );
  menu.addAction( KIcon("edit-copy"), i18n("Clone Agent"), this, SLOT(cloneAgent()) );
  menu.addSeparator();
  menu.addMenu( mSyncMenu );
  menu.addAction( KIcon("dialog-cancel"), i18n("Abort Activity"), this, SLOT(abortAgent()) );
  menu.addAction( KIcon("system-reboot"), i18n("Restart Agent"), this, SLOT(restartAgent()) );  //FIXME: Is using system-reboot icon here a good idea?
  menu.addAction( KIcon("network-disconnect"), i18n("Toggle Online/Offline"), this, SLOT(toggleOnline()) );
  menu.addAction( KIcon(""), i18n("Show task list"), this, SLOT(showTaskList()) );
  menu.addAction( KIcon(""), i18n("Show change-notification log"), this, SLOT(showChangeNotifications()) );
  menu.addMenu( mConfigMenu );
  menu.addAction( KIcon("list-remove"), i18n("Remove Agent"), this, SLOT(removeAgent()) );
  menu.exec( ui.instanceWidget->mapToGlobal( pos ) );
}

void AgentWidget::resizeEvent( QResizeEvent *event )
{
  ui.detailsBox->setVisible( event->size().height() > 400 );
}

