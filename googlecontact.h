#ifndef GOOGLECONTACT_H
#define GOOGLECONTACT_H

#include <QContact>
#include <QDomElement>

class GoogleContact : public QContact 
{
  public:
    GoogleContact (const QDomNode &node) ;
    GoogleContact();
    GoogleContact (QContact &contact);
    ~GoogleContact();
    
    QStringList googleGroups() const;
    void setGoogleGroup(const QStringList &groups);
    void addGoogleGroup(const QString &group);
    
    QString googleId() const;
    void setGoogleId(const QString &id);
  private:
    enum GoogleField // Corresponding to http://code.google.com/apis/gdata/elements.html#gdContactKind 
    {
      Id,
      Title,
      Content,
      
      Email,
      IM,
      PhoneNumber,
      PostalAddress,
      Organisation,
      
      GroupMembershipInfo
    };
    
    void parse(const QDomNode &node);
    
    static bool hashesFilled;
    static void fillHashes();
    static QHash<QString, GoogleField> fields;
    static QHash<QString, PhoneType> phoneTypes;
    static QHash<QString, Location> postalAddressTypes;

    QStringList googleGroupList;;
};

#endif // GOOGLECONTACT_H
