#include <QDebug>
#include <QApplication>

#include "googlesession.h"
#include "googlesync.h"

GoogleSync::GoogleSync(QObject *parent)
: QObject(parent), session(new GoogleSession(this)), inProgress(false)
{
  connect(session, SIGNAL(error(GoogleSession::Error, QString)), SLOT(googleError(GoogleSession::Error, QString)));
  connect(session, SIGNAL(authenticated()), SLOT(googleAuth()));
  connect(session, SIGNAL(groupsFetched(QHash<QString, QString>)), SLOT(googleGroups(QHash<QString, QString>)));
  connect(session, SIGNAL(contactsFetched(QList<QContact>)), SLOT(googleContacts(QList<QContact>)));
  connect(session, SIGNAL(stateChanged(GoogleSession::State )), SIGNAL(stateChanged(GoogleSession::State)) );
}

GoogleSync::~GoogleSync()
{
}

bool GoogleSync::start(const QString &login, const QString &passwd, bool setskip)
{
  if (inProgress)
    return false;
  inProgress = true;
  skip = setskip;
  session->login(login, passwd);
}

void GoogleSync::googleError(GoogleSession::Error err, const QString &reason)
{
  qCritical() << "Google error";
  QApplication::instance()->exit(1);
}

void GoogleSync::googleAuth()
{
  qDebug() << "Google authenticated";
  session->fetchGroups();
}

void GoogleSync::googleGroups(QHash<QString, QString> groups)
{
  qDebug() << "Groups!";
  groupMap = groups;
  for (QHash<QString, QString>::iterator it=groupMap.begin(); it!=groupMap.end(); it++)
  {
    qDebug() << it.key() << "->" << it.value();
  }
  session->setGroups(groups);
  session->updateGroups();
  
  session->fetchContacts();
}

void GoogleSync::googleContacts(QList<QContact> contacts)
{
  qDebug() << "Got contacts. Updating" << skip;
  session->updateContacts(contacts,skip);
  qDebug() << "Terminating";
  QApplication::instance()->exit(0);
}
