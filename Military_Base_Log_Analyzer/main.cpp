#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <deque>
#include <algorithm>
#include <memory>
#include <compare>
#include <stdexcept>
#include <optional>
using namespace std::literals;
#endif /* __PROGTEST__ */

#include <cstdint>
#include <climits>
#include <limits>

class CTimeStamp
{
  public:
    CTimeStamp()
  : m_Year(0), m_Month(0), m_Day(0), m_Hour(0), m_Minute(0)
{}
    CTimeStamp ( int year, int month, int day, int hour, int minute )
      : m_Year ( year )
      , m_Month ( month )
      , m_Day ( day )
      , m_Hour ( hour )
      , m_Minute ( minute )
    {
    }
    bool operator < ( const CTimeStamp & x ) const
    {
      if ( m_Year != x . m_Year ) return m_Year < x . m_Year;
      if ( m_Month != x . m_Month ) return m_Month < x . m_Month;
      if ( m_Day != x . m_Day ) return m_Day < x . m_Day;
      if ( m_Hour != x . m_Hour ) return m_Hour < x . m_Hour;
      return m_Minute < x . m_Minute;
    }
    bool operator == ( const CTimeStamp & x ) const
    {
      return m_Year == x . m_Year
          && m_Month == x . m_Month
          && m_Day == x . m_Day
          && m_Hour == x . m_Hour
          && m_Minute == x . m_Minute;
    }

    bool operator > ( const CTimeStamp & x ) const { return x < *this; }
    bool operator <= ( const CTimeStamp & x ) const { return !(x < *this); }
    bool operator >= ( const CTimeStamp & x ) const { return !(*this < x); } 

    bool isValid() const{
      if (m_Year < 1 || m_Month < 1 || m_Month > 12) return false;
      if (m_Day < 1 || m_Day > mdays(m_Year, m_Month)) return false;
      if (m_Hour < 0 || m_Hour > 23) return false;
      if (m_Minute < 0 || m_Minute > 59) return false;
      return true;
    }

    long long toMinutes() const{
      int y = m_Year;
      unsigned m = (unsigned)m_Month;
      unsigned d = (unsigned)m_Day;
      y -= m <= 2;
      const int era = (y >= 0 ? y : y-399) / 400;
      const unsigned yoe = (unsigned)(y - era * 400);
      const unsigned doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d - 1;
      const unsigned doe = yoe*365 + yoe/4 - yoe/100 + doy;
      long long days = (long long)era * 146097 + (long long)doe - 719468;
      return days * 24LL * 60LL + (long long)m_Hour * 60LL + m_Minute;
    }
  private:
    int   m_Year;
    int   m_Month;
    int   m_Day;
    int   m_Hour;
    int   m_Minute;

    static int mdays(int year, int month)
    {
      const int md[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
      if (month != 2) return md[month];
      return md[2] + (isLeapYear(year) ? 1 : 0);
    }

    static bool isLeapYear(int y)
    {
      if (y % 400 == 0) return true;
      if (y % 100 == 0) return false;
      return (y % 4) == 0;
    }

};

// moved helpers are now member functions of CTimeStamp

class CAuditFilter
{
  public:
    CAuditFilter(const std::string & zone)
      : m_Zone(zone) {}

    CAuditFilter & notBefore(int y, int m, int d, int h, int mi)
    {
      m_From = CTimeStamp(y,m,d,h,mi);
      m_HasFrom = true;
      return *this;
    }

    CAuditFilter & notAfter(int y, int m, int d, int h, int mi)
    {
      m_To = CTimeStamp(y,m,d,h,mi);
      m_HasTo = true;
      return *this;
    }

    const std::string & zone() const { return m_Zone; }

    bool hasFrom() const { return m_HasFrom; }
    bool hasTo() const { return m_HasTo; }
    const CTimeStamp & from() const { return m_From; }
    const CTimeStamp & to() const { return m_To; }

  private:
    std::string m_Zone;

    bool m_HasFrom = false;
    bool m_HasTo = false;

    CTimeStamp m_From;
    CTimeStamp m_To;
};

// Forward declarations
using RawData = std::map<std::string, std::vector<std::pair<CTimeStamp,std::string>>>;

class CGraph {
  public:
    void addEdge(const std::string &a, const std::string &b, int time) {
      m_graph[a][b] = time;
      m_graph[b][a] = time;
      // clear cache because graph changed
      m_distanceCache.clear();
    }

    const std::map<std::string,int> & getDistancesFrom(const std::string & src) const {
  auto it = m_distanceCache.find(src);
  if (it != m_distanceCache.end()) return it->second;

      std::map<std::string,int> best;
      using State = std::pair<int,std::string>;
      std::priority_queue<State, std::vector<State>, std::greater<State>> pq;
      pq.push({0, src});
      best[src] = 0;

      while (!pq.empty()) {
        std::pair<int,std::string> top = pq.top();
        pq.pop();
        int t = top.first;
        const std::string currentZone = top.second;
        if (best[currentZone] != t) continue;
        auto git = m_graph.find(currentZone);
        if (git == m_graph.end()) continue;
        for (auto &p : git->second) {
          const std::string &nextZone = p.first;
          int travelTime = p.second;
          int newDistance = t + travelTime;
          if (!best.count(nextZone) || newDistance < best[nextZone]) {
            best[nextZone] = newDistance;
            pq.push({newDistance, nextZone});
          }
        }
      }
      m_distanceCache[src] = std::move(best);
      return m_distanceCache[src];
    }

  private:
  std::map<std::string, std::map<std::string,int>> m_graph;
  mutable std::map<std::string, std::map<std::string,int>> m_distanceCache;
};

class CLogParser {
  public:
  static RawData parseLogFile(const std::string & filename) {
      std::ifstream ifs(filename, std::ios::binary);
      if (!ifs) throw std::runtime_error("open fail");

      RawData raw;

      while (true) {
        if (!ifs) throw std::runtime_error("invalid format");

        int c = ifs.peek();
        if (c == EOF) break;

        char magic[4];
        ifs.read(magic, 4);
        if (ifs.gcount() != 4)
          throw std::runtime_error("invalid format");

        if (!std::memcmp(magic, "TEXT", 4)) {
          std::string zone;
          int count;
          if (!(ifs >> zone >> count) || count < 0 || count > 1000000)
            throw std::runtime_error("invalid format");
          ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          parseText(ifs, raw, zone, count);
        } else if (!std::memcmp(magic, "\x49\x49\x49\x49", 4)) {
          parseBinaryLE(ifs, raw);
        } else if (!std::memcmp(magic, "\x4D\x4D\x4D\x4D", 4)) {
          parseBinaryBE(ifs, raw);
        } else
          throw std::runtime_error("format");
      }

      if (!ifs.eof())
        throw std::runtime_error("invalid format");

      return raw;
    }

  private:
    static void parseText(std::ifstream &ifs, RawData &raw, const std::string &zone, int count){
      std::string line;
      for (int i = 0; i < count; i++) {
        if (!std::getline(ifs, line))
          throw std::runtime_error("invalid format");
        if (line.empty()) { i--; continue; }
        std::istringstream iss(line);
        int y, m, d, h, mi; char c1, c2, c3;
        if (!(iss >> y >> c1 >> m >> c2 >> d >> h >> c3 >> mi))
          throw std::runtime_error("invalid format");
        if (c1 != '-' || c2 != '-' || c3 != ':') throw std::runtime_error("invalid format");
        std::string person; std::getline(iss, person);
        if (!person.empty() && person[0] == ' ') person.erase(0,1);
        if (person.empty()) throw std::runtime_error("invalid format");
        CTimeStamp ts(y,m,d,h,mi);
        if (!ts.isValid()) throw std::runtime_error("invalid datetime");
        raw[person].push_back({ts, zone});
      }
    }

    static void parseBinaryLE(std::ifstream &ifs, RawData &raw) {
      uint16_t len; ifs.read(reinterpret_cast<char*>(&len), 2);
      std::string zone(len, '\0');
      if (len > 0)
        ifs.read(&zone[0], len);
      if (!ifs) throw std::runtime_error("invalid format");
      uint32_t count; ifs.read(reinterpret_cast<char*>(&count), 4);
      if (!ifs) throw std::runtime_error("invalid format");
        for (uint32_t i = 0; i < count; ++i) {
          uint32_t dt; ifs.read(reinterpret_cast<char*>(&dt), 4);
          if (!ifs) throw std::runtime_error("invalid format");
          int mi = dt & 0x3F;
          int h  = (dt >> 6) & 0x1F;
          int d  = (dt >> 11) & 0x1F;
          int m  = (dt >> 16) & 0x0F;
          int y  = (dt >> 20);
          uint16_t nlen; ifs.read(reinterpret_cast<char*>(&nlen), 2);
          if (!ifs) throw std::runtime_error("invalid format");
          std::string person(nlen, '\0');
          if (nlen > 0)
            ifs.read(&person[0], nlen);
          if (!ifs) throw std::runtime_error("invalid format");
          CTimeStamp ts(y,m,d,h,mi); 
          if (!ts.isValid()) throw std::runtime_error("invalid datetime");
          raw[person].push_back({ts, zone});
        }
    }

    static void parseBinaryBE(std::ifstream &ifs, RawData &raw) {
      auto read16 = [&](uint16_t &x){ unsigned char b[2]; ifs.read((char*)b,2); if (!ifs) throw std::runtime_error("invalid format"); x = (b[0]<<8) | b[1]; };
      auto read32 = [&](uint32_t &x){ unsigned char b[4]; ifs.read((char*)b,4); if (!ifs) throw std::runtime_error("invalid format"); x = (b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3]; };
      uint16_t len; read16(len);
      std::string zone(len, '\0');
      if (len > 0)
        ifs.read(&zone[0], len);
      if (!ifs) throw std::runtime_error("invalid format");
      uint32_t count; read32(count);
      for (uint32_t i = 0; i < count; ++i) {
        uint32_t dt; read32(dt);
        int mi = dt & 0x3F;
        int h  = (dt >> 6) & 0x1F;
        int d  = (dt >> 11) & 0x1F;
        int m  = (dt >> 16) & 0x0F;
        int y  = (dt >> 20);
        uint16_t nlen; read16(nlen);
        std::string person(nlen, '\0');
        if (nlen > 0)
          ifs.read(&person[0], nlen);
        if (!ifs) throw std::runtime_error("invalid format");
        CTimeStamp ts(y,m,d,h,mi); if (!ts.isValid()) throw std::runtime_error("invalid datetime");
        raw[person].push_back({ts, zone});
      }
    }
};

class CVisitorLog
{
  public:
    struct Interval {
      std::string from;
      std::string to;
      long long start; // minutes
      long long end;   // minutes
      bool overlaps(long long qFrom, long long qTo) const {
        return !(end < qFrom || start > qTo);
      }
    };

    void setGraph(const CGraph * g) { m_graphRef = g; }

    void buildIntervals(const RawData & raw) {
      for (const auto & personRec : raw) {
        const std::string & person = personRec.first;
        const auto & records = personRec.second;
        auto sortedRecords = records;
        std::sort(sortedRecords.begin(), sortedRecords.end(), [](const auto &a, const auto &b){ return a.first < b.first; });
        std::vector<Interval> intervals;
        for (size_t i = 0; i < sortedRecords.size(); ) {
          Interval interval;
          interval.from = sortedRecords[i].second;
          CTimeStamp tin = sortedRecords[i].first;
          if (i + 1 < sortedRecords.size()) {
            interval.to = sortedRecords[i+1].second;
            CTimeStamp tout = sortedRecords[i+1].first;
            interval.start = tin.toMinutes();
            interval.end = tout.toMinutes();
            i += 2;
          } else {
            interval.to = interval.from;
            interval.start = tin.toMinutes();
            interval.end = CTimeStamp(9999,12,31,23,59).toMinutes();
            i += 1;
          }
          intervals.push_back(interval);
        }
        m_personIntervals[person] = std::move(intervals);
      }
    }

    bool canBeInZone(const Interval &interval,
                 const std::map<std::string,int> &distToTarget,
                 long long fromMin, long long toMin) const {
        auto itIn  = distToTarget.find(interval.from);
        auto itOut = distToTarget.find(interval.to);

        if (itIn == distToTarget.end() || itOut == distToTarget.end())
            return false;

        long long enter = interval.start + itIn->second;
        long long exit  = interval.end   - itOut->second;

        if (enter > exit)
            return false;

        long long L = std::max(enter, fromMin);
        long long R = std::min(exit, toMin);

        return L <= R;
    }

    std::set<std::string> search(const CAuditFilter &filter) const {
        std::set<std::string> result;

        long long fromMin = LLONG_MIN;
        long long toMin   = LLONG_MAX;

        if (filter.hasFrom()) fromMin = filter.from().toMinutes();
        if (filter.hasTo())   toMin   = filter.to().toMinutes();

        const auto &target = filter.zone();
        const auto &distToTarget = m_graphRef->getDistancesFrom(target);

        for (const auto & personIntervals : m_personIntervals)
        {
            const std::string & person = personIntervals.first;
            const std::vector<Interval> & intervals = personIntervals.second;
            for (const auto &iv : intervals)
            {
                if (canBeInZone(iv, distToTarget, fromMin, toMin))
                {
                    result.insert(person);
                    break;
                }
            }
        }

        return result;
    }

  private:
  mutable std::map<std::string, std::map<std::string,int>> m_cachedFrom;
  std::map<std::string, std::vector<Interval>> m_personIntervals;
  const CGraph * m_graphRef = nullptr;
};

class CMilBase
{
  public:
    CMilBase         ()
    {
    }
    void readBase    ( const std::string & baseFilename ){
      std::ifstream ifs(baseFilename);
      if (!ifs) throw std::runtime_error("Failed to open base file");
      std::string line, fromZone, toZone; int travelTime;
      while (std::getline(ifs, line)){
        std::istringstream iss(line);
        if (!(iss >> fromZone >> toZone >> travelTime))
          throw std::runtime_error("invalid format");
        if (travelTime < 1 || travelTime > 9)
          throw std::runtime_error("invalid format");
        m_graph.addEdge(fromZone,toZone,travelTime);
      }
    }

    CVisitorLog processLog(const std::string & logFilename) {
      auto rawLogs = CLogParser::parseLogFile(logFilename);
      CVisitorLog result;
      result.buildIntervals(rawLogs);
      result.setGraph(&m_graph);
      return result;
    }
  private:
    CGraph m_graph;
};

#ifndef __PROGTEST__
void basicTests ( const CVisitorLog & log )
{
  assert ( log . search ( CAuditFilter ( "headquarters" ) )
           == ( std::set<std::string> { "Alice Cooper", "George Peterson", "Henry Montgomery", "Jane Bush", "John Smith", "Tim Cook", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "Jane Bush", "John Smith", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notAfter ( 2026, 3, 10, 8, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 11, 12, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery", "Jane Bush", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 0 ) . notAfter ( 2026, 3, 10, 13, 0 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "Jane Bush", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 5 ) . notAfter ( 2026, 3, 10, 9, 5 ) )
           == ( std::set<std::string> { "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 6 ) . notAfter ( 2026, 3, 10, 9, 6 ) )
           == ( std::set<std::string> { "Henry Montgomery", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 24 ) . notAfter ( 2026, 3, 10, 9, 24 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 25 ) . notAfter ( 2026, 3, 10, 9, 25 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2024, 2, 1, 0, 0 ) . notAfter ( 2024, 3, 31, 0, 0 ) )
           == ( std::set<std::string> { "Robert Smith", "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2025, 2, 1, 0, 0 ) . notAfter ( 2025, 3, 31, 0, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "privateParking" )  )
           == ( std::set<std::string> { "<classified>" } ) );
}

int main ()
{
  class CMilBase b;
  b . readBase ( "base.txt" );

  for ( const char * fn : std::initializer_list<const char *> { "in1.log", "in2.log", "in3.log" } )
    basicTests ( b . processLog ( fn ) );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */