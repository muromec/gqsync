#ifndef GOOGLESESSION_H
#define GOOGLESESSION_H

#include <QObject>
#include <QString>
#include <QHash>

#include <QDomElement>
#include <QContact>
#include "googlecontact.h"

class QHttp;

class GoogleSession: public QObject
{
  Q_OBJECT
  public:
    enum State
    {
      Invalid,
      Authenticating,
      Authenticated,
      FetchingGroups,
      FetchingContacts,
      UpdatingGroups,
      UpdatingContacts,
    };
    
    enum Error
    {
      AuthenticationFailed,
      InvalidState
    };
    
    GoogleSession(QObject *parent = NULL);
    virtual ~GoogleSession();
    
    State state() const;
    int updateContacts(QList<QContact>&, bool skip);
    int updateGroups();
    static QString stateName(State s);
    
  public slots:
    void login(const QString &login, const QString &passwd);   
    void fetchGroups();
    void fetchContacts();
    void setGroups(QHash<QString, QString> groups);
    QContact merge(QContact contact, GoogleContact gContact);
    
  signals:
    void error(GoogleSession::Error error, QString reason);
    void authenticated();
    void groupsFetched(QHash<QString, QString> groups);
    void contactsFetched(QList<QContact> contacts);
    void stateChanged(GoogleSession::State);
    
  private slots:
    void httpResult(int id, bool error);
  private:
    void parseGroups(const QString &xml, QHash<QString, QString> &groupMap);
    void parseContacts(const QString &xml, QList<QContact> &contacts);
    void authResult(bool errorFlag);
    void groupsResult(bool errorFlag);
    void contactsResult(bool errorFlag);
    
    void setState(State newState);
    
    QHttp *http;    
    int authReqId;    
    int groupsFetchId;
    int contactsFetchId;
    QString authKey;    
    State m_state;
    QHash<QString, QString> groups;

};

#endif // GOOGLESESSION_H
