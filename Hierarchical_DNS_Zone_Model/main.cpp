#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
#include <compare>
#include "ipaddress.h"
using namespace std::literals;
#endif /* __PROGTEST__ */

class CEntry
{
  public:
    explicit                   CEntry   ( std::string name )
      : m_Name ( std::move ( name ) )
    {
    }
    virtual                    ~CEntry   ( void ) = default;
    const std::string        & name     ( void ) const { return m_Name; }
    virtual std::string        type     ( void ) const = 0;
    virtual void               print    ( std::ostream & os ) const = 0; // no trailing \n
    virtual std::shared_ptr<CEntry> clone ( void ) const = 0;
    virtual bool               equals   ( const CEntry & other ) const = 0;
    virtual bool               isZone   ( void ) const { return false; }

  protected:
    std::string m_Name;
};

class CRecA : public CEntry
{
  public:
    CRecA ( std::string name, const CIPv4 & addr )
      : CEntry ( std::move ( name ) ), m_Addr ( addr )
    {
    }
    std::string type ( void ) const override { return "A"; }
    void print ( std::ostream & os ) const override
    {
      os << m_Name << " " << type () << " " << m_Addr;
    }
    std::shared_ptr<CEntry> clone ( void ) const override
    {
      return std::make_shared<CRecA> ( *this );
    }
    bool equals ( const CEntry & other ) const override
    {
      auto o = dynamic_cast<const CRecA *> ( &other );
      return o && m_Name == o -> m_Name && m_Addr == o -> m_Addr;
    }
    friend std::ostream & operator << ( std::ostream & os, const CRecA & x )
    {
      x . print ( os );
      return os;
    }
  private:
    CIPv4       m_Addr;
};

class CRecAAAA : public CEntry
{
  public:
    CRecAAAA ( std::string name, const CIPv6 & addr )
      : CEntry ( std::move ( name ) ), m_Addr ( addr )
    {
    }
    std::string type ( void ) const override { return "AAAA"; }
    void print ( std::ostream & os ) const override
    {
      os << m_Name << " " << type () << " " << m_Addr;
    }
    std::shared_ptr<CEntry> clone ( void ) const override
    {
      return std::make_shared<CRecAAAA> ( *this );
    }
    bool equals ( const CEntry & other ) const override
    {
      auto o = dynamic_cast<const CRecAAAA *> ( &other );
      return o && m_Name == o -> m_Name && m_Addr == o -> m_Addr;
    }
    friend std::ostream & operator << ( std::ostream & os, const CRecAAAA & x )
    {
      x . print ( os );
      return os;
    }
  private:
    CIPv6       m_Addr;
};

class CRecMX : public CEntry
{
  public:
    CRecMX ( std::string name, std::string host, int pref )
      : CEntry ( std::move ( name ) ), m_Host ( std::move ( host ) ), m_Pref ( pref )
    {
    }
    std::string type ( void ) const override { return "MX"; }
    void print ( std::ostream & os ) const override
    {
      os << m_Name << " " << type () << " " << m_Pref << " " << m_Host;
    }
    std::shared_ptr<CEntry> clone ( void ) const override
    {
      return std::make_shared<CRecMX> ( *this );
    }
    bool equals ( const CEntry & other ) const override
    {
      auto o = dynamic_cast<const CRecMX *> ( &other );
      return o && m_Name == o -> m_Name && m_Host == o -> m_Host && m_Pref == o -> m_Pref;
    }
    friend std::ostream & operator << ( std::ostream & os, const CRecMX & x )
    {
      x . print ( os );
      return os;
    }
  private:
    std::string m_Host;
    int         m_Pref;
};

class CRecCNAME : public CEntry
{
  public:
    CRecCNAME ( std::string name, std::string target )
      : CEntry ( std::move ( name ) ), m_Target ( std::move ( target ) )
    {
    }
    std::string type ( void ) const override { return "CNAME"; }
    void print ( std::ostream & os ) const override
    {
      os << m_Name << " " << type () << " " << m_Target;
    }
    std::shared_ptr<CEntry> clone ( void ) const override
    {
      return std::make_shared<CRecCNAME> ( *this );
    }
    bool equals ( const CEntry & other ) const override
    {
      auto o = dynamic_cast<const CRecCNAME *> ( &other );
      return o && m_Name == o -> m_Name && m_Target == o -> m_Target;
    }
    friend std::ostream & operator << ( std::ostream & os, const CRecCNAME & x )
    {
      x . print ( os );
      return os;
    }
  private:
    std::string m_Target;
};

class CRecSPF : public CEntry
{
  public:
    explicit CRecSPF ( std::string name )
      : CEntry ( std::move ( name ) )
    {
    }
    std::string type ( void ) const override { return "SPF"; }
    CRecSPF & add ( std::string addr )
    {
      m_Addrs . push_back ( std::move ( addr ) );
      return *this;
    }
    void print ( std::ostream & os ) const override
    {
      os << m_Name << " " << type () << " ";
      for ( size_t i = 0; i < m_Addrs . size (); ++i )
      {
        if ( i )
          os << ", ";
        os << m_Addrs[i];
      }
    }
    std::shared_ptr<CEntry> clone ( void ) const override
    {
      return std::make_shared<CRecSPF> ( *this );
    }
    bool equals ( const CEntry & other ) const override
    {
      auto o = dynamic_cast<const CRecSPF *> ( &other );
      return o && m_Name == o -> m_Name && m_Addrs == o -> m_Addrs;
    }
    friend std::ostream & operator << ( std::ostream & os, const CRecSPF & x )
    {
      x . print ( os );
      return os;
    }
  private:
    std::vector<std::string> m_Addrs;
};

class CZone : public CEntry
{
  public:
    explicit CZone ( std::string name )
      : CEntry ( std::move ( name ) )
    {
    }
    CZone ( const CZone & other )
      : CEntry ( other . m_Name )
    {
      for ( const auto & it : other . m_Order )
        m_Order . push_back ( it -> clone () );
      rebuildIndex ();
    }
    CZone & operator = ( const CZone & other )
    {
      if ( this == &other )
        return *this;
      m_Name = other . m_Name;
      m_Order . clear ();
      for ( const auto & it : other . m_Order )
        m_Order . push_back ( it -> clone () );
      rebuildIndex ();
      return *this;
    }

    std::string type ( void ) const override { return "ZONE"; }
    bool isZone ( void ) const override { return true; }
    void print ( std::ostream & os ) const override { (void) os; }
    std::shared_ptr<CEntry> clone ( void ) const override
    {
      return std::make_shared<CZone> ( *this );
    }
    bool equals ( const CEntry & other ) const override
    {
      auto o = dynamic_cast<const CZone *> ( &other );
      return o && m_Name == o -> m_Name;
    }

    bool add ( const CRecA & x )      { return addEntry ( x ); }
    bool add ( const CRecAAAA & x )   { return addEntry ( x ); }
    bool add ( const CRecMX & x )     { return addEntry ( x ); }
    bool add ( const CRecCNAME & x )  { return addEntry ( x ); }
    bool add ( const CRecSPF & x )    { return addEntry ( x ); }
    bool add ( const CZone & x )      { return addEntry ( x ); }
    bool add ( const CEntry & x )     { return addEntry ( x ); }

    bool del ( const CRecA & x )      { return delEntry ( x ); }
    bool del ( const CRecAAAA & x )   { return delEntry ( x ); }
    bool del ( const CRecMX & x )     { return delEntry ( x ); }
    bool del ( const CRecCNAME & x )  { return delEntry ( x ); }
    bool del ( const CRecSPF & x )    { return delEntry ( x ); }
    bool del ( const CZone & x )      { return delEntry ( x ); }
    bool del ( const CEntry & x )     { return delEntry ( x ); }

    class CSearchResult
    {
      public:
        CSearchResult ( void ) = default;
        explicit CSearchResult ( std::vector<std::shared_ptr<CEntry>> items )
          : m_Items ( std::move ( items ) )
        {
        }
        size_t size ( void ) const { return m_Items . size (); }
        CEntry & operator [] ( size_t idx )
        {
          if ( idx >= m_Items . size () )
            throw std::out_of_range ( "CZone::search result index" );
          return *m_Items[idx];
        }
        const CEntry & operator [] ( size_t idx ) const
        {
          if ( idx >= m_Items . size () )
            throw std::out_of_range ( "CZone::search result index" );
          return *m_Items[idx];
        }
        friend std::ostream & operator << ( std::ostream & os, const CSearchResult & r )
        {
          for ( const auto & it : r . m_Items )
          {
            if ( it -> isZone () )
              os << dynamic_cast<const CZone &> ( *it );
            else
            {
              it -> print ( os );
              os << "\n";
            }
          }
          return os;
        }
      private:
        std::vector<std::shared_ptr<CEntry>> m_Items;
    };

    CSearchResult search ( std::string q ) const
    {
      if ( ! q . empty () && q . back () == '.' )
        q . pop_back ();
      if ( q . empty () )
        return CSearchResult ();

      std::vector<std::string> parts;
      size_t pos = 0;
      while ( true )
      {
        size_t dot = q . find ( '.', pos );
        if ( dot == std::string::npos )
        {
          parts . push_back ( q . substr ( pos ) );
          break;
        }
        parts . push_back ( q . substr ( pos, dot - pos ) );
        pos = dot + 1;
      }

      const CZone * cur = this;
      if ( parts . size () > 1 )
      {
        for ( size_t i = parts . size () - 1; i >= 1; --i )
        {
          const CZone * next = cur -> findSubZone ( parts[i] );
          if ( ! next )
            return CSearchResult ();
          cur = next;
          if ( i == 1 )
            break;
        }
      }
      return cur -> searchLocal ( parts[0] );
    }

    friend std::ostream & operator << ( std::ostream & os, const CZone & z )
    {
      os << z . m_Name << "\n";
      z . printChildren ( os, " " );
      return os;
    }

  private:
    using TList = std::list<std::shared_ptr<CEntry>>;
    using TIt   = TList::iterator;

    void rebuildIndex ( void )
    {
      m_Index . clear ();
      for ( auto it = m_Order . begin (); it != m_Order . end (); ++it )
        m_Index[(*it) -> name ()] . push_back ( it );
    }

    std::vector<TIt> * bucket ( const std::string & n )
    {
      auto it = m_Index . find ( n );
      if ( it == m_Index . end () )
        return nullptr;
      return &it -> second;
    }
    const std::vector<TIt> * bucket ( const std::string & n ) const
    {
      auto it = m_Index . find ( n );
      if ( it == m_Index . end () )
        return nullptr;
      return &it -> second;
    }

    bool nameHasType ( const std::string & n, const std::string & t ) const
    {
      auto b = bucket ( n );
      if ( ! b ) return false;
      for ( const auto & it : *b )
        if ( (*it) -> type () == t )
          return true;
      return false;
    }
    bool nameHasZone ( const std::string & n ) const
    {
      auto b = bucket ( n );
      if ( ! b ) return false;
      for ( const auto & it : *b )
        if ( (*it) -> isZone () )
          return true;
      return false;
    }

    const CZone * findSubZone ( const std::string & n ) const
    {
      auto b = bucket ( n );
      if ( ! b ) return nullptr;
      for ( const auto & it : *b )
        if ( (*it) -> isZone () )
          return dynamic_cast<const CZone *> ( it -> get () );
      return nullptr;
    }

    CSearchResult searchLocal ( const std::string & n ) const
    {
      std::vector<std::shared_ptr<CEntry>> res;
      auto b = bucket ( n );
      if ( ! b )
        return CSearchResult ();
      res . reserve ( b -> size () );
      for ( const auto & it : *b )
        res . push_back ( *it );
      return CSearchResult ( std::move ( res ) );
    }

    bool addEntry ( const CEntry & x )
    {
      auto p = x . clone ();
      const std::string & n = p -> name ();
      const bool xIsZone = p -> isZone ();
      const std::string t = p -> type ();

      // collision rules (CNAME/zone exclusivity)
      if ( xIsZone )
      {
        if ( bucket ( n ) != nullptr )
          return false;
      }
      else if ( t == "CNAME" )
      {
        if ( bucket ( n ) != nullptr )
          return false;
      }
      else
      {
        if ( nameHasType ( n, "CNAME" ) || nameHasZone ( n ) )
          return false;
      }

      // duplicate record check (same type+payload)
      auto b = bucket ( n );
      if ( b )
        for ( const auto & it : *b )
          if ( (*it) -> equals ( *p ) )
            return false;

      m_Order . push_back ( std::move ( p ) );
      auto itNew = std::prev ( m_Order . end () );
      m_Index[n] . push_back ( itNew );
      return true;
    }

    bool delEntry ( const CEntry & x )
    {
      const std::string & n = x . name ();
      auto itIdx = m_Index . find ( n );
      if ( itIdx == m_Index . end () )
        return false;

      auto & vec = itIdx -> second;
      bool found = false;
      for ( size_t i = 0; i < vec . size (); )
      {
        auto itList = vec[i];
        if ( (*itList) -> equals ( x ) )
        {
          m_Order . erase ( itList );
          vec . erase ( vec . begin () + i );
          found = true;
        }
        else
          ++i;
      }
      if ( vec . empty () )
        m_Index . erase ( itIdx );
      return found;
    }

    void printChildren ( std::ostream & os, const std::string & prefix ) const
    {
      for ( auto it = m_Order . begin (); it != m_Order . end (); ++it )
      {
        auto next = it;
        ++next;
        if ( next == m_Order . end () )
          os << prefix << "\\- ";
        else
          os << prefix << "+- ";

        const auto & e = *it;
        if ( e -> isZone () )
        {
          const CZone & z = dynamic_cast<const CZone &> ( *e );
          os << z . m_Name << "\n";
          z . printChildren ( os, prefix + ( next == m_Order . end () ? "   " : "|  " ) );
        }
        else
        {
          e -> print ( os );
          os << "\n";
        }
      }
    }

    TList m_Order;
    std::unordered_map<std::string, std::vector<TIt>> m_Index;
};

inline std::ostream & operator << ( std::ostream & os, const CEntry & e )
{
  if ( e . isZone () )
    return os << dynamic_cast<const CZone &> ( e );
  e . print ( os );
  return os;
}

#ifndef __PROGTEST__
int main ()
{
  std::ostringstream oss;

  CZone z0 ( "fit" );
  assert ( z0 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z0 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.158" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.159" ) ) ) == true );
  assert ( z0 . add ( CRecCNAME ( "pririz", "sto.fit.cvut.cz." ) ) == true );
  assert ( z0 . add ( CRecSPF ( "courses" ) . add ( "ip4:147.32.232.128/25" ) . add ( "ip4:147.32.232.64/26" ) ) == true );
  assert ( z0 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:1:2:3:4" ) ) ) == true );
  assert ( z0 . add ( CRecMX ( "courses", "relay.fit.cvut.cz.", 0 ) ) == true );
  assert ( z0 . add ( CRecMX ( "courses", "relay2.fit.cvut.cz.", 10 ) ) == true );
  oss . str ( "" );
  oss << z0;
  assert ( oss . str () == 
    "fit\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- courses MX 0 relay.fit.cvut.cz.\n"
    " \\- courses MX 10 relay2.fit.cvut.cz.\n" );
  assert ( z0 . search ( "progtest" ) . size () == 3 );
  oss . str ( "" );
  oss << z0 . search ( "progtest" );
  assert ( oss . str () == 
    "progtest A 147.32.232.142\n"
    "progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "progtest AAAA 2001:718:2:2902:1:2:3:4\n" );
  assert ( z0 . del ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.122" ) ) ) == true );
  oss . str ( "" );
  oss << z0;
  assert ( oss . str () == 
    "fit\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.159\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- courses MX 0 relay.fit.cvut.cz.\n"
    " +- courses MX 10 relay2.fit.cvut.cz.\n"
    " \\- courses A 147.32.232.122\n" );
  assert ( z0 . search ( "courses" ) . size () == 6 );
  oss . str ( "" );
  oss << z0 . search ( "courses" );
  assert ( oss . str () == 
    "courses A 147.32.232.158\n"
    "courses A 147.32.232.159\n"
    "courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "courses MX 0 relay.fit.cvut.cz.\n"
    "courses MX 10 relay2.fit.cvut.cz.\n"
    "courses A 147.32.232.122\n" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 0 ];
  assert ( oss . str () == "courses A 147.32.232.158" );
  assert ( z0 . search ( "courses" ) [ 0 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 0 ] . type () == "A" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 1 ];
  assert ( oss . str () == "courses A 147.32.232.159" );
  assert ( z0 . search ( "courses" ) [ 1 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 1 ] . type () == "A" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 2 ];
  assert ( oss . str () == "courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26" );
  assert ( z0 . search ( "courses" ) [ 2 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 2 ] . type () == "SPF" );
  try
  {
    oss . str ( "" );
    oss << z0 . search ( "courses" ) [ 10 ];
    assert ( "No exception thrown!" == nullptr );
  }
  catch ( const std::out_of_range & e )
  {
  }
  catch ( ... )
  {
    assert ( "Invalid exception thrown!" == nullptr );
  }
  dynamic_cast<const CRecAAAA &> ( z0 . search ( "progtest" ) [ 1 ] );
  CZone z1 ( "fit2" );
  z1 . add ( z0 . search ( "progtest" ) [ 2 ] );
  z1 . add ( z0 . search ( "progtest" ) [ 0 ] );
  z1 . add ( z0 . search ( "progtest" ) [ 1 ] );
  z1 . add ( z0 . search ( "courses" ) [ 2 ] );
  oss . str ( "" );
  oss << z1;
  assert ( oss . str () == 
    "fit2\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n" );
  dynamic_cast<const CRecA &> ( z1 . search ( "progtest" ) [ 1 ] );

  CZone z10 ( "fit" );
  assert ( z10 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z10 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z10 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.144" ) ) ) == true );
  assert ( z10 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 10 ) ) == true );
  assert ( z10 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == false );
  assert ( z10 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.140" ) ) ) == false );
  assert ( z10 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z10 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == false );
  assert ( z10 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 20 ) ) == true );
  assert ( z10 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 10 ) ) == false );
  assert ( z10 . add ( CRecCNAME ( "pririz", "sto.fit.cvut.cz." ) ) == true );
  assert ( z10 . add ( CRecCNAME ( "pririz", "stojedna.fit.cvut.cz." ) ) == false );
  assert ( z10 . add ( CRecA ( "pririz", CIPv4 ( "147.32.232.111" ) ) ) == false );
  assert ( z10 . add ( CRecCNAME ( "progtest", "progtestbak.fit.cvut.cz." ) ) == false );
  assert ( z10 . add ( CZone ( "test" ) ) == true );
  assert ( z10 . add ( CZone ( "pririz" ) ) == false );
  assert ( z10 . add ( CRecA ( "test", CIPv4 ( "147.32.232.232" ) ) ) == false );
  oss . str ( "" );
  oss << z10;
  assert ( oss . str () == 
    "fit\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- progtest A 147.32.232.144\n"
    " +- progtest MX 10 relay.fit.cvut.cz.\n"
    " +- progtest MX 20 relay.fit.cvut.cz.\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " \\- test\n" );
  assert ( z10 . search ( "progtest" ) . size () == 4 );
  oss . str ( "" );
  oss << z10 . search ( "progtest" );
  assert ( oss . str () == 
    "progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "progtest A 147.32.232.144\n"
    "progtest MX 10 relay.fit.cvut.cz.\n"
    "progtest MX 20 relay.fit.cvut.cz.\n" );
  assert ( z10 . search ( "courses" ) . size () == 0 );
  oss . str ( "" );
  oss << z10 . search ( "courses" );
  assert ( oss . str () == "" );

  CZone z20 ( "<ROOT ZONE>" );
  CZone z21 ( "cz" );
  CZone z22 ( "cvut" );
  CZone z23 ( "fit" );
  assert ( z23 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z23 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z23 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.158" ) ) ) == true );
  assert ( z23 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z23 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.159" ) ) ) == true );
  assert ( z23 . add ( CRecCNAME ( "pririz", "sto.fit.cvut.cz." ) ) == true );
  assert ( z23 . add ( CRecSPF ( "courses" ) . add ( "ip4:147.32.232.128/25" ) . add ( "ip4:147.32.232.64/26" ) ) == true );
  CZone z24 ( "fel" );
  assert ( z24 . add ( CRecA ( "www", CIPv4 ( "147.32.80.2" ) ) ) == true );
  assert ( z24 . add ( CRecAAAA ( "www", CIPv6 ( "1:2:3:4:5:6:7:8" ) ) ) == true );
  assert ( z22 . add ( z23 ) == true );
  assert ( z22 . add ( z24 ) == true );
  assert ( z21 . add ( z22 ) == true );
  assert ( z20 . add ( z21 ) == true );
  assert ( z23 . add ( CRecA ( "www", CIPv4 ( "147.32.90.1" ) ) ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  oss . str ( "" );
  oss << z21;
  assert ( oss . str () == 
    "cz\n"
    " \\- cvut\n"
    "    +- fit\n"
    "    |  +- progtest A 147.32.232.142\n"
    "    |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "    |  +- courses A 147.32.232.158\n"
    "    |  +- courses A 147.32.232.160\n"
    "    |  +- courses A 147.32.232.159\n"
    "    |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "    |  \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "    \\- fel\n"
    "       +- www A 147.32.80.2\n"
    "       \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  oss . str ( "" );
  oss << z22;
  assert ( oss . str () == 
    "cvut\n"
    " +- fit\n"
    " |  +- progtest A 147.32.232.142\n"
    " |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |  +- courses A 147.32.232.158\n"
    " |  +- courses A 147.32.232.160\n"
    " |  +- courses A 147.32.232.159\n"
    " |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |  \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " \\- fel\n"
    "    +- www A 147.32.80.2\n"
    "    \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  oss . str ( "" );
  oss << z23;
  assert ( oss . str () == 
    "fit\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " \\- www A 147.32.90.1\n" );
  oss . str ( "" );
  oss << z24;
  assert ( oss . str () == 
    "fel\n"
    " +- www A 147.32.80.2\n"
    " \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  assert ( z20 . search ( "progtest.fit.cvut.cz" ) . size () == 2 );
  oss . str ( "" );
  oss << z20 . search ( "progtest.fit.cvut.cz" );
  assert ( oss . str () == 
    "progtest A 147.32.232.142\n"
    "progtest AAAA 2001:718:2:2902:0:1:2:3\n" );
  assert ( z20 . search ( "fit.cvut.cz" ) . size () == 1 );
  oss . str ( "" );
  oss << z20 . search ( "fit.cvut.cz" );
  assert ( oss . str () == 
    "fit\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n" );
  assert ( dynamic_cast<CZone &> ( z20 . search ( "fit.cvut.cz" ) [0] ) . add ( z20 . search ( "fel.cvut.cz" ) [0] ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |  \\- fel\n"
    "       |     +- www A 147.32.80.2\n"
    "       |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  assert ( dynamic_cast<CZone &> ( z20 . search ( "fit.cvut.cz" ) [0] ) . add ( z20 . search ( "cz" ) [0] ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |  +- fel\n"
    "       |  |  +- www A 147.32.80.2\n"
    "       |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |  \\- cz\n"
    "       |     \\- cvut\n"
    "       |        +- fit\n"
    "       |        |  +- progtest A 147.32.232.142\n"
    "       |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |  +- courses A 147.32.232.158\n"
    "       |        |  +- courses A 147.32.232.160\n"
    "       |        |  +- courses A 147.32.232.159\n"
    "       |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |  \\- fel\n"
    "       |        |     +- www A 147.32.80.2\n"
    "       |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        \\- fel\n"
    "       |           +- www A 147.32.80.2\n"
    "       |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  assert ( dynamic_cast<CZone &> ( z20 . search ( "fit.cvut.cz.fit.cvut.cz" ) [0] ) . add ( z20 . search ( "cz" ) [0] ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |  +- fel\n"
    "       |  |  +- www A 147.32.80.2\n"
    "       |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |  \\- cz\n"
    "       |     \\- cvut\n"
    "       |        +- fit\n"
    "       |        |  +- progtest A 147.32.232.142\n"
    "       |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |  +- courses A 147.32.232.158\n"
    "       |        |  +- courses A 147.32.232.160\n"
    "       |        |  +- courses A 147.32.232.159\n"
    "       |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |  +- fel\n"
    "       |        |  |  +- www A 147.32.80.2\n"
    "       |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |  \\- cz\n"
    "       |        |     \\- cvut\n"
    "       |        |        +- fit\n"
    "       |        |        |  +- progtest A 147.32.232.142\n"
    "       |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |        |  +- courses A 147.32.232.158\n"
    "       |        |        |  +- courses A 147.32.232.160\n"
    "       |        |        |  +- courses A 147.32.232.159\n"
    "       |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |        |  +- fel\n"
    "       |        |        |  |  +- www A 147.32.80.2\n"
    "       |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        |  \\- cz\n"
    "       |        |        |     \\- cvut\n"
    "       |        |        |        +- fit\n"
    "       |        |        |        |  +- progtest A 147.32.232.142\n"
    "       |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |        |        |  +- courses A 147.32.232.158\n"
    "       |        |        |        |  +- courses A 147.32.232.160\n"
    "       |        |        |        |  +- courses A 147.32.232.159\n"
    "       |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |        |        |  \\- fel\n"
    "       |        |        |        |     +- www A 147.32.80.2\n"
    "       |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        |        \\- fel\n"
    "       |        |        |           +- www A 147.32.80.2\n"
    "       |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        \\- fel\n"
    "       |        |           +- www A 147.32.80.2\n"
    "       |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        \\- fel\n"
    "       |           +- www A 147.32.80.2\n"
    "       |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  assert ( dynamic_cast<CZone &> ( z20 . search ( "fit.cvut.cz.fit.cvut.cz" ) [0] ) . del ( CZone ( "fel" ) ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |  +- fel\n"
    "       |  |  +- www A 147.32.80.2\n"
    "       |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |  \\- cz\n"
    "       |     \\- cvut\n"
    "       |        +- fit\n"
    "       |        |  +- progtest A 147.32.232.142\n"
    "       |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |  +- courses A 147.32.232.158\n"
    "       |        |  +- courses A 147.32.232.160\n"
    "       |        |  +- courses A 147.32.232.159\n"
    "       |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |  \\- cz\n"
    "       |        |     \\- cvut\n"
    "       |        |        +- fit\n"
    "       |        |        |  +- progtest A 147.32.232.142\n"
    "       |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |        |  +- courses A 147.32.232.158\n"
    "       |        |        |  +- courses A 147.32.232.160\n"
    "       |        |        |  +- courses A 147.32.232.159\n"
    "       |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |        |  +- fel\n"
    "       |        |        |  |  +- www A 147.32.80.2\n"
    "       |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        |  \\- cz\n"
    "       |        |        |     \\- cvut\n"
    "       |        |        |        +- fit\n"
    "       |        |        |        |  +- progtest A 147.32.232.142\n"
    "       |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |        |        |  +- courses A 147.32.232.158\n"
    "       |        |        |        |  +- courses A 147.32.232.160\n"
    "       |        |        |        |  +- courses A 147.32.232.159\n"
    "       |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |        |        |  \\- fel\n"
    "       |        |        |        |     +- www A 147.32.80.2\n"
    "       |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        |        \\- fel\n"
    "       |        |        |           +- www A 147.32.80.2\n"
    "       |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        \\- fel\n"
    "       |        |           +- www A 147.32.80.2\n"
    "       |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        \\- fel\n"
    "       |           +- www A 147.32.80.2\n"
    "       |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  CZone z25 ( z20 );
z22 = z20;
  assert ( z20 . add ( CZone ( "sk" ) ) == true );
  assert ( z25 . add ( CZone ( "au" ) ) == true );
  assert ( z22 . add ( CZone ( "de" ) ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " +- cz\n"
    " |  \\- cvut\n"
    " |     +- fit\n"
    " |     |  +- progtest A 147.32.232.142\n"
    " |     |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |  +- courses A 147.32.232.158\n"
    " |     |  +- courses A 147.32.232.160\n"
    " |     |  +- courses A 147.32.232.159\n"
    " |     |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |  +- fel\n"
    " |     |  |  +- www A 147.32.80.2\n"
    " |     |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |  \\- cz\n"
    " |     |     \\- cvut\n"
    " |     |        +- fit\n"
    " |     |        |  +- progtest A 147.32.232.142\n"
    " |     |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |  +- courses A 147.32.232.158\n"
    " |     |        |  +- courses A 147.32.232.160\n"
    " |     |        |  +- courses A 147.32.232.159\n"
    " |     |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |  \\- cz\n"
    " |     |        |     \\- cvut\n"
    " |     |        |        +- fit\n"
    " |     |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |  +- fel\n"
    " |     |        |        |  |  +- www A 147.32.80.2\n"
    " |     |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |  \\- cz\n"
    " |     |        |        |     \\- cvut\n"
    " |     |        |        |        +- fit\n"
    " |     |        |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |        |  \\- fel\n"
    " |     |        |        |        |     +- www A 147.32.80.2\n"
    " |     |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |        \\- fel\n"
    " |     |        |        |           +- www A 147.32.80.2\n"
    " |     |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        \\- fel\n"
    " |     |        |           +- www A 147.32.80.2\n"
    " |     |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        \\- fel\n"
    " |     |           +- www A 147.32.80.2\n"
    " |     |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     \\- fel\n"
    " |        +- www A 147.32.80.2\n"
    " |        \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " \\- sk\n" );
  oss . str ( "" );
  oss << z22;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " +- cz\n"
    " |  \\- cvut\n"
    " |     +- fit\n"
    " |     |  +- progtest A 147.32.232.142\n"
    " |     |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |  +- courses A 147.32.232.158\n"
    " |     |  +- courses A 147.32.232.160\n"
    " |     |  +- courses A 147.32.232.159\n"
    " |     |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |  +- fel\n"
    " |     |  |  +- www A 147.32.80.2\n"
    " |     |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |  \\- cz\n"
    " |     |     \\- cvut\n"
    " |     |        +- fit\n"
    " |     |        |  +- progtest A 147.32.232.142\n"
    " |     |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |  +- courses A 147.32.232.158\n"
    " |     |        |  +- courses A 147.32.232.160\n"
    " |     |        |  +- courses A 147.32.232.159\n"
    " |     |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |  \\- cz\n"
    " |     |        |     \\- cvut\n"
    " |     |        |        +- fit\n"
    " |     |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |  +- fel\n"
    " |     |        |        |  |  +- www A 147.32.80.2\n"
    " |     |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |  \\- cz\n"
    " |     |        |        |     \\- cvut\n"
    " |     |        |        |        +- fit\n"
    " |     |        |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |        |  \\- fel\n"
    " |     |        |        |        |     +- www A 147.32.80.2\n"
    " |     |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |        \\- fel\n"
    " |     |        |        |           +- www A 147.32.80.2\n"
    " |     |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        \\- fel\n"
    " |     |        |           +- www A 147.32.80.2\n"
    " |     |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        \\- fel\n"
    " |     |           +- www A 147.32.80.2\n"
    " |     |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     \\- fel\n"
    " |        +- www A 147.32.80.2\n"
    " |        \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " \\- de\n" );
  oss . str ( "" );
  oss << z25;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " +- cz\n"
    " |  \\- cvut\n"
    " |     +- fit\n"
    " |     |  +- progtest A 147.32.232.142\n"
    " |     |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |  +- courses A 147.32.232.158\n"
    " |     |  +- courses A 147.32.232.160\n"
    " |     |  +- courses A 147.32.232.159\n"
    " |     |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |  +- fel\n"
    " |     |  |  +- www A 147.32.80.2\n"
    " |     |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |  \\- cz\n"
    " |     |     \\- cvut\n"
    " |     |        +- fit\n"
    " |     |        |  +- progtest A 147.32.232.142\n"
    " |     |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |  +- courses A 147.32.232.158\n"
    " |     |        |  +- courses A 147.32.232.160\n"
    " |     |        |  +- courses A 147.32.232.159\n"
    " |     |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |  \\- cz\n"
    " |     |        |     \\- cvut\n"
    " |     |        |        +- fit\n"
    " |     |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |  +- fel\n"
    " |     |        |        |  |  +- www A 147.32.80.2\n"
    " |     |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |  \\- cz\n"
    " |     |        |        |     \\- cvut\n"
    " |     |        |        |        +- fit\n"
    " |     |        |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |        |  \\- fel\n"
    " |     |        |        |        |     +- www A 147.32.80.2\n"
    " |     |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |        \\- fel\n"
    " |     |        |        |           +- www A 147.32.80.2\n"
    " |     |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        \\- fel\n"
    " |     |        |           +- www A 147.32.80.2\n"
    " |     |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        \\- fel\n"
    " |     |           +- www A 147.32.80.2\n"
    " |     |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     \\- fel\n"
    " |        +- www A 147.32.80.2\n"
    " |        \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " \\- au\n" );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
