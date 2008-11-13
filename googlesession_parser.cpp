#include "googlesession.h"

#include <QString>
#include <QHash>

#include <QDomDocument>
#include <QDomElement>

#include <QDebug>

#include "googlecontact.h"

void GoogleSession::parseGroups(const QString &xml, QHash<QString, QString> &groupMap)
{
  QDomDocument doc;
  doc.setContent(xml, true);
  
  //qDebug() << "\n" << doc.toString(2);

  QDomElement element = doc.documentElement();

  for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
  {
    if (n.nodeName() == "entry")
    {
      QString id;
      QString title;
      for (QDomNode nn = n.firstChild(); !nn.isNull(); nn = nn.nextSibling())
      {
        QDomElement ee = nn.toElement();

        // parsing
        QString tag  = ee.tagName();
        QString text = ee.text();

        if (tag == "id")
          id = text;
        else if (tag == "title")
          title = text;
      }
      groupMap[id] = title;
    }
  }
}

void GoogleSession::parseContacts(const QString &xml, QList<QContact> &contacts)
{
  QDomDocument doc;
  doc.setContent(xml, true);

  //qDebug() << doc.toString(2);

  QDomElement element = doc.documentElement();

  for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling())
  {
    if (n.nodeName() == "entry")
    {
      contacts.append(GoogleContact(n));
    }
  }
}
