#include <googlecontact.h>
#include <QDebug>

#include <QDomElement>

bool GoogleContact::hashesFilled = false;
QHash<QString, GoogleContact::GoogleField> GoogleContact::fields;
QHash<QString, GoogleContact::PhoneType> GoogleContact::phoneTypes;
QHash<QString, GoogleContact::Location> GoogleContact::postalAddressTypes;

void GoogleContact::fillHashes()
{
  if (hashesFilled)
    return;
  
  fields["id"] = Id;
  fields["title"] = Title;
  fields["content"] = Content;
  fields["email"] = Email;
  fields["im"] = IM;
  fields["phoneNumber"] = PhoneNumber;
  fields["postalAddress"] = PostalAddress;
  fields["organisation"] = Organisation;
  fields["groupMembershipInfo"] = GroupMembershipInfo;
  
  // OtherPhone, Mobile, Fax, Pager, HomePager and VOIP are restricted
  // see libraries/qtopiapim 
  phoneTypes["http://schemas.google.com/g/2005#fax"] = QContact::Fax;  
  phoneTypes["http://schemas.google.com/g/2005#home"] = QContact::HomePhone;  
  phoneTypes["http://schemas.google.com/g/2005#home_fax"] = QContact::HomeFax;   
  phoneTypes["http://schemas.google.com/g/2005#mobile"] = QContact::HomeMobile;   
  phoneTypes["http://schemas.google.com/g/2005#other"] = QContact::HomeVOIP;  
  phoneTypes["http://schemas.google.com/g/2005#pager"] = QContact::BusinessPager;  
  phoneTypes["http://schemas.google.com/g/2005#work"] = QContact::BusinessPhone;   
  phoneTypes["http://schemas.google.com/g/2005#work_fax"] = QContact::BusinessFax;
  
  postalAddressTypes["http://schemas.google.com/g/2005#home"] = QContact::Home;
  postalAddressTypes["http://schemas.google.com/g/2005#work"] = QContact::Business;
  postalAddressTypes["http://schemas.google.com/g/2005#other"] = QContact::Other;
}

GoogleContact::GoogleContact() 
  : QContact() 
{
}


GoogleContact::GoogleContact(const QDomNode& node) 
  : QContact() 
{
  parse(node);
}

GoogleContact::GoogleContact(QContact& contact):
  QContact(contact)
{
}

void GoogleContact::parse(const QDomNode &node)
{
  fillHashes();
  qDebug() << "\nGoogleContact: parsing XML node...";
  for(QDomNode nn = node.firstChild(); !nn.isNull(); nn = nn.nextSibling()) 
  {
    QDomElement field = nn.toElement();
    //qDebug() << "Tag" << field.tagName();
    if (fields.contains(field.tagName()))
    {
      switch (fields[field.tagName()])
      {
        case Id:
          setGoogleId(field.text());
          qDebug() << "Id:" << field.text();
          break;
        case Title:
          setFirstName(field.text());
          qDebug() << "Title:" << field.text();
          break;
        case Content:
          setNotes(field.text());
          qDebug() << "Content:" << field.text();
          break;
        case Email:
          insertEmail(field.attribute("address"));
          qDebug() << "Email:" << field.attribute("address");
          break;
        case IM:
          qDebug() << "Got IM of type" << field.attribute("protocol") << "-" << field.attribute("address") << "(don't know what to do with it)";
          break;
        case PhoneNumber:
        {
          PhoneType pt = phoneTypes[field.attribute("rel", "http://schemas.google.com/g/2005#other")];
          setPhoneNumber(pt, field.text());
          qDebug() << "Phone number:" << field.text() << "type" << pt;
        }
          break;
        case PostalAddress:
        {
          Location lc = postalAddressTypes[field.attribute("rel", "http://schemas.google.com/g/2005#other")];
          QContactAddress addr;
          addr.street = field.text(); // FIXME need real parsing here
          setAddress(lc, addr);
          qDebug() << "Postal Address:" << field.text() << "type" << lc;
        }
          break;
        case Organisation:
          setOffice(field.text());
          qDebug() << "Organisation:" << field.text();
          break;
        case GroupMembershipInfo:
          QList<QString> mCategories = categories();
          mCategories  << field.attribute("href");
          setCategories(mCategories);
          qDebug() << "Group:" << field.attribute("href");
          break;
      }
    }
  }
}

GoogleContact::~GoogleContact() 
{
};



QString GoogleContact::googleId() const
{
  return customField("googleId");
}

void GoogleContact::setGoogleId(const QString &id)
{
  setCustomField("googleId", id);
}
