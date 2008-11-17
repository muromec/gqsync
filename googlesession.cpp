#include <QHash>
#include <QString>
#include <QStringList>

#include <QHttp>
#include <QHttpResponseHeader>
#include <QHttpRequestHeader>

#include <QDebug>

#include "googlesession.h"

//#define USER_AGENT "GQSync/1.0 (gzip)"
#define USER_AGENT "GQSync/1.0"

GoogleSession::GoogleSession(QObject *parent)
: QObject(parent), http(NULL), m_state(Invalid), 
  authReqId(-1), groupsFetchId(-1), contactsFetchId(-1)
{
}

GoogleSession::~GoogleSession()
{
}

void GoogleSession::setState(State newState)
{
  static QString states[] = {"Invalid","Authenticating","Authenticated","FetchingGroups","FetchingContacts"};
  qDebug() << "GoogleSession: state" << states[m_state] << "->" << states[newState];
  m_state = newState;
}

GoogleSession::State GoogleSession::state() const
{
  return m_state;
}

void GoogleSession::login(const QString &login, const QString &passwd)
{
  if (m_state!=Invalid)
  {
    emit error(InvalidState, "");
    return;
  }
  
  if (!http)
  {
    http = new QHttp(this);
    connect(http, SIGNAL(requestFinished(int, bool)), SLOT(httpResult(int, bool)));
  }
  
  QHttpRequestHeader header("POST", "https://www.google.com/accounts/ClientLogin");
  http->setHost("google.com");
  
  header.setValue("Host", "google.com");
  header.setValue( "User-Agent", USER_AGENT);
  header.setContentType("application/x-www-form-urlencoded");
  
  QString queryString = QString("Email=%1&Passwd=%2&accountType=GOOGLE&service=cp").arg(login).arg(passwd);
  
  authReqId = http->request(header, queryString.toUtf8());
  setState(Authenticating);    
}

void GoogleSession::httpResult(int id, bool errorFlag)
{
  if (id==authReqId) // Authentication result
  {
    if (errorFlag)
    {
      qDebug() << "Auth http error" << http->errorString();
      setState(Invalid);
      emit error(AuthenticationFailed, http->errorString());
    }
    else
    {
      QString resp = http->readAll(); 
      //qDebug() << resp;
      QStringList keys = resp.split("\n");
      QHash<QString, QString> keyMap;
      for (QStringList::iterator it = keys.begin(); it!=keys.end(); it++)
      {
        int sep = it->indexOf('=');
        QString key = it->left(sep);
        QString value = it->right(it->length()-sep-1);
        keyMap[key] = value;
        //qDebug() << key << value;
      }
      if (http->lastResponse().statusCode()==200) // OK
      {
        if (keyMap.contains("Auth"))
        {
          authKey = keyMap["Auth"];
          qDebug() << "Authenticated" << authKey;
          setState(Authenticated);
          emit authenticated();
        }
        else
        {
          setState(Invalid);
          emit error(AuthenticationFailed, "No Auth key");
        }
      }
      else
      {
        qDebug() << "ERROR Response header:" << http->lastResponse().statusCode() << http->lastResponse().reasonPhrase();
        qDebug() << "ERROR reason" << keyMap["Error"];
        setState(Invalid);
        emit error(AuthenticationFailed, keyMap["Error"]);
      }
    }
  }
  else if (id==groupsFetchId) // Groups fetch result
  {
    QByteArray respData = http->readAll();

    qDebug() << "Groups Response header:" << http->lastResponse().statusCode() << http->lastResponse().reasonPhrase();
    qDebug() << "Groups HTTP headers\n" << http->lastResponse().toString();
    // FIXME: Need status code check here 
    QString resp;
    if (http->lastResponse().value("content-encoding")=="gzip")
    {
      QByteArray respInflated = qUncompress(respData);
      resp = QString::fromUtf8(respInflated.constData()); 
    }
    else
      resp = QString::fromUtf8(respData.constData()); 
    QHash<QString, QString> groupMap;
    parseGroups(resp, groupMap);
    setState(Authenticated);
    emit groupsFetched(groupMap);
  }
  else if (id==contactsFetchId) // Groups fetch result
  {
    QString resp = QString::fromUtf8(http->readAll().constData()); 
    qDebug() << "Contacts Response header:" << http->lastResponse().statusCode() << http->lastResponse().reasonPhrase();
    QList<QContact> contacts;
    parseContacts(resp, contacts);
    setState(Authenticated);
  }
  else
    qWarning() << "GoogleSession: Invalid response id" << id << "error:" << errorFlag;
}

void GoogleSession::fetchGroups()
{
  if (m_state!=Authenticated)
  {
    emit error(InvalidState, "");
    return;
  }
  
  QHttpRequestHeader header("GET", "/m8/feeds/groups/default/full");
  header.setValue("Authorization",   "GoogleLogin auth="+authKey);
  header.setValue("Host",            "www.google.com");
  header.setValue("Accept-Encoding", "gzip");
  header.setValue("User-Agent",      USER_AGENT);

  http->setHost("www.google.com");
  groupsFetchId = http->request(header);
  setState(FetchingGroups);
  qDebug() << "Fetching groups...";
}

void GoogleSession::setGroups(QHash<QString, QString> groups)
{
  this->groups = groups;
}

void GoogleSession::fetchContacts()
{
  if (m_state!=Authenticated)
  {
    emit error(InvalidState, "");
    return;
  }
  
  QHttpRequestHeader header("GET", "/m8/feeds/contacts/default/full");
  header.setValue("Authorization",   "GoogleLogin auth="+authKey);
  header.setValue("Host",            "www.google.com");
  header.setValue("Accept-Encoding", "gzip");
  header.setValue("User-Agent",      USER_AGENT);

  http->setHost("www.google.com");
  contactsFetchId = http->request(header);
  setState(FetchingContacts);
  qDebug() << "Fetching conacts...";
}
