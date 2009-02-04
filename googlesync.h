#ifndef GOOGLESYNC_H
#define GOOGLESYNC_H

#include <QObject>
#include <QString>
#include <QHash>

#include "googlesession.h"

class GoogleSync: public QObject
{
  Q_OBJECT
  public:
    GoogleSync(QObject *parent = NULL);
    virtual ~GoogleSync();
  
    bool start(const QString &login, const QString &passwd, bool setskip);
    
  private slots:
    void googleError(GoogleSession::Error err, const QString &reason);
    void googleAuth();
    void googleGroups(QHash<QString, QString> groups);
    void googleContacts(QList<QContact> contacts);
  private:
    GoogleSession *session;
    bool inProgress;
    bool skip;
    QHash<QString, QString> groupMap;

  signals:
    void stateChanged(GoogleSession::State state);
};

#endif
