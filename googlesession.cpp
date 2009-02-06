#include <QHash>
#include <QString>
#include <QStringList>

#include <QHttp>
#include <QHttpResponseHeader>
#include <QHttpRequestHeader>

#include <QContactModel>
#include <QCategoryManager>

#include <QDebug>

#include "googlesession.h"
#include "gzip.h"

#define USER_AGENT_GZ "GQSync/1.0 (gzip)"
#define USER_AGENT "GQSync/1.0"
static QString stateNames[] = {
  "Invalid",
  "Authenticating",
  "Authenticated",
  "Fetching groups",
  "Fetching contacts",
  "Updating groups",
  "Updating contacts",
};

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
  qDebug() << "GoogleSession: state" << stateNames[m_state] << "->" << stateNames[newState];
  m_state = newState;
  emit stateChanged(m_state);
}

QString GoogleSession::stateName(State s) {
  return stateNames[s];
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
    authResult(errorFlag);
  else if (id==groupsFetchId) // Groups fetch result
    groupsResult(errorFlag);  
  else if (id==contactsFetchId) // Groups fetch result
    contactsResult(errorFlag); 
  else
    qDebug() << "GoogleSession: Invalid response id" << id << "error:" << errorFlag;
}

void GoogleSession::authResult(bool errorFlag)
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
void GoogleSession::groupsResult(bool errorFlag)
{
    QByteArray respData = http->readAll();

    qDebug() << "Groups Response header:" << http->lastResponse().statusCode() << http->lastResponse().reasonPhrase();
    qDebug() << "Groups HTTP headers\n" << http->lastResponse().toString();
    QString resp;
    if (http->lastResponse().value("content-encoding")=="gzip")
    {
      char *data = inf(respData.constData(), respData.size() );
      resp = QString::fromUtf8(data);
      free(data);
    }
    else
      resp = QString::fromUtf8(respData.constData()); 

    QHash<QString, QString> groupMap;
    parseGroups(resp, groupMap);

    setState(Authenticated);
    emit groupsFetched(groupMap);
  }

void GoogleSession::contactsResult(bool errorFlag)
{
    QByteArray respData = http->readAll();
    qDebug() << "Contacts Response header:" << http->lastResponse().statusCode() << http->lastResponse().reasonPhrase();
    qDebug() << "Contacts HTTP headers\n" << http->lastResponse().toString(); 

    QString resp;
    if (http->lastResponse().value("content-encoding")=="gzip")
    {
      char *data = inf(respData.constData(), respData.size() );
      resp = QString::fromUtf8(data);
      free(data);
    }
    else
      resp = QString::fromUtf8(respData.constData()); 

    QList<QContact> contacts;
    parseContacts(resp, contacts);
    setState(Authenticated);
    emit contactsFetched(contacts);
}   

void GoogleSession::fetchGroups()
{
  if (m_state!=Authenticated)
  {
    emit error(InvalidState, "");
    return;
  }
  
  QHttpRequestHeader header("GET", "/m8/feeds/groups/default/full?max-results=10000");
  header.setValue("Authorization",   "GoogleLogin auth="+authKey);
  header.setValue("Host",            "www.google.com");
  header.setValue("Accept-Encoding", "gzip");
  header.setValue("User-Agent",      USER_AGENT_GZ);

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
  
  QHttpRequestHeader header("GET", "/m8/feeds/contacts/default/full?max-results=10000");
  header.setValue("Authorization",   "GoogleLogin auth="+authKey);
  header.setValue("Host",            "www.google.com");
  header.setValue("Accept-Encoding", "gzip");
  header.setValue("User-Agent",      USER_AGENT_GZ);

  http->setHost("www.google.com");
  contactsFetchId = http->request(header);
  setState(FetchingContacts);
  qDebug() << "Fetching conacts...";
}

int GoogleSession::updateContacts(QList<QContact> &contacts, bool skip) {

  setState(UpdatingContacts);
  QContactModel filter;

  for (int i = 0; i < contacts.size(); ++i) {
    QContact gContact = contacts.at(i);

    // skip contacts with no phonenumbers
    if (skip && (! gContact.phoneNumbers().size() ))
      continue;

    filter.setFilter(gContact.label());

    // single match. mering
    if (filter.count() == 1) {
      filter.updateContact( merge(filter.contact(0), gContact) );
    } 
    // no match. saving directly
    else if (filter.count() == 0) {
      filter.addContact(gContact);
    }
  }

  setState(Authenticated);
}

QContact GoogleSession::merge(QContact contact, GoogleContact gContact)
{

  qDebug() << "==";
  qDebug() << contact.label() << gContact.label();

  // merge email lists
  QStringList gl = gContact.emailList(); 
  QStringList el =  contact.emailList();

  for (int i=0; i<gl.size(); ++i) {
    if ( el.contains( gl.at(i) ) )
      qDebug() << "already have email" <<  gl.at(i) ;
    else
      contact.insertEmail(  gl.at(i)  );
  }

  // merge phone number lists
  QMap<QContact::PhoneType, QString> nums  = contact.phoneNumbers(); 
  QMap<QContact::PhoneType, QString> gNums = gContact.phoneNumbers(); 


  // iterate google data
  QMapIterator<QContact::PhoneType, QString> gNumIt(gNums);

  while (gNumIt.hasNext()) {
    gNumIt.next();
    QString phone             = gNumIt.value();
    QContact::PhoneType type  = gNumIt.key();

    if (! nums.values().contains( phone  ) ) {
      gContact.setPhoneNumber(type,  phone);
      qDebug () << "adding phone" << phone << "of type" << type;
    } else {

       // iterate qtopia data
       QMapIterator<QContact::PhoneType, QString> it(nums);
       while (it.hasNext()) {
              it.next();
              qDebug() << "contacts has phone" << it.value() << "of type" << it.key();
              qDebug() << "comparing to google" << phone << "of type" << type;
              if (it.value() ==  phone && it.key() != type ) {

                bool updatedef = (contact.defaultPhoneNumber() == phone);

                nums.remove(it.key() );
                nums.remove(type); // FIXME
                nums.insert(type,  phone);
                contact.setPhoneNumbers(nums);

                if (updatedef)
                  contact.setDefaultPhoneNumber(type);

                qDebug() << "replaced phone of type" <<  type << phone << updatedef;
                break;
              }
       }
      qDebug () << "skipping phone" << phone << type ;

    }
  }

  QStringList googleGroupList = gContact.categories();


  for (int i=0; i<googleGroupList.size(); ++i) {

    QString googleGroupId   = googleGroupList.at(i);
    QString googleGroupName = groups[googleGroupId];
    QString googleGroupQId   = "category." + googleGroupName;


    qDebug() << "Group id" <<googleGroupId << "name" << googleGroupName ;

    if (googleGroupName.isEmpty())
      continue;



    QList<QString> qGroupList = contact.categories();
    if (! qGroupList.contains(googleGroupQId) )  {
      qGroupList << googleGroupQId;

      qDebug() << "Adding group" << googleGroupName ;

      contact.setCategories(qGroupList);

    } else {
      qDebug() << "Skipping group" << googleGroupName;
    }

    qDebug() << "group count" << qGroupList.count();

  }

  qDebug() << "\n";
  return contact;

}

int GoogleSession::updateGroups()
{
  setState(UpdatingGroups);
  QCategoryManager cats("Address Book");

  QHashIterator<QString, QString> it(groups);

  while (it.hasNext()) {
    it.next();

    QString googleGroupId   = it.key();
    QString googleGroupName = it.value();
    QString googleGroupQId   = "category." + googleGroupName;

    if (!cats.exists( googleGroupQId  ) ) {
      cats.ensureSystemCategory(googleGroupQId, googleGroupName);
    } 
  
  }

  setState(Authenticated);

}
    
