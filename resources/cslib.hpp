  #include <unordered_map> // Faster lookups
#include <filesystem> // Finding and moving files and folders in the Anti36Local folder
#include <algorithm> // Finding elements in a container
#include <iostream> // Printing out messages and std::string (and everything else that comes with it)
#include <fstream> // Logging
#include <cstdint> // Cooler numbers (small numbers supported by stl)
#include <chrono>
#include <deque> // Containers with continuous memory addresses

// #include "httplib.h" // HTTP server
#include "json.hpp" // JSON parser


namespace cslib {
  // Custom lib

  // Typedefs
  using uint = unsigned int;
  using tint = uint8_t;


  class DualOutput { public:
    std::ofstream file;
    DualOutput(const std::string &path) {
      file.open(path, std::ios::app);
    }
    template <typename T>
    DualOutput& operator<<(const T &data) {
      std::cout << data;
      file << data;
      return *this;
    }
    inline std::string ask() {
      std::string result;
      std::getline(std::cin, result);
      return result;
    }
  };


  static std::deque<std::string> separate(const std::string &str, const char delimiter) {
    /*
      This function takes a string and a delimiter and splits the string into
      a deque of strings. The delimiter is used to split the string into different parts.

      Note:
        - The delimiter is not included in the result
        - If the string or delimiter is empty, an empty deque is returned
        - If between two delimiters is nothing, an empty string is added to the deque
        - If the string ends with a delimiter, an empty string is added to the deque
        - If delimiter is not found, the whole string is added to the deque
    */

    std::deque<std::string> result;
    std::string temp;

    if (str.empty() or delimiter == '\0') {
      return result;
    }

    for (char c : str) {
        if (c == delimiter) {
            result.push_back(temp);
            temp.clear();
        } else {
            temp += c;
        }
    }

    result.push_back(temp);

    return result;
  }


  class TimeStamp { public:
    enum Weekday : tint {
      MONDAY = 'M',
      TUESDAY = 'U', // U for tUesday
      WEDNESDAY = 'W',
      THURSDAY = 'T',
      FRIDAY = 'F',
      SATURDAY = 'A', // A for sAturday
      SUNDAY = 'S',
      ERROR_DAY = 'E'
    };

    Weekday weekday = ERROR_DAY;
    tint day;
    tint month;
    tint year; // Last two digits of the year (e.g. 2022 -> 22)
    tint hour;
    tint minute;
    tint second;

    void update() {
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
      struct tm* timeinfo = std::localtime(&currentTime);
  
      switch (timeinfo->tm_wday) {
        case 0: weekday = SUNDAY; break;
        case 1: weekday = MONDAY; break;
        case 2: weekday = TUESDAY; break;
        case 3: weekday = WEDNESDAY; break;
        case 4: weekday = THURSDAY; break;
        case 5: weekday = FRIDAY; break;
        case 6: weekday = SATURDAY; break;
        default: weekday = ERROR_DAY; break;
      }
      day = timeinfo->tm_mday;
      month = timeinfo->tm_mon + 1;
      year = std::stoi(std::to_string(timeinfo->tm_year + 1900).substr(2, 2));
      hour = timeinfo->tm_hour;
      minute = timeinfo->tm_min;
      second = timeinfo->tm_sec;
    }

    inline bool operator<(const TimeStamp &other) const {
      if (year < other.year) {return true;}
      if (year > other.year) {return false;}
      if (month < other.month) {return true;}
      if (month > other.month) {return false;}
      if (day < other.day) {return true;}
      if (day > other.day) {return false;}
      if (hour < other.hour) {return true;}
      if (hour > other.hour) {return false;}
      if (minute < other.minute) {return true;}
      if (minute > other.minute) {return false;}
      if (second < other.second) {return true;}
      return false;
    }
  
    static constexpr Weekday determine_day_of_weeky(uint day, uint month, uint year) {
      // Zeller's Congruence (chatgpt be my goat fr fr)
      uint q = day; // Day of the month
      uint m = month; // Month
      uint K = year % 100; // Year of the century
      uint J = year / 100; // Zero-based century
      if (m < 3) {
        m += 12;
        K--;
      }
      uint h = (q + (13 * (m + 1)) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
      switch (h) {
        case 0: return SATURDAY;
        case 1: return SUNDAY;
        case 2: return MONDAY;
        case 3: return TUESDAY;
        case 4: return WEDNESDAY;
        case 5: return THURSDAY;
        case 6: return FRIDAY;
        default: return ERROR_DAY;
      }
    }
  
    TimeStamp() {update();};
    TimeStamp(uint d, uint m, uint y, uint h, uint min, uint s) {
      this->day = d;
      this->month = m;
      this->year = std::stoi(std::to_string(y).substr(2, 2));
      this->hour = h;
      this->minute = min;
      this->second = s;
      weekday = determine_day_of_weeky(*this);
    }
  };
  

  class VirtualPath { public:
    /*
      Specialized-lightweight wrapper for std::filesystem::path
      VirtualPath aint a real path, but a representation of it
    */
    enum Type {
      FILE = 'F',
      DICT = 'D',
    };
  
    std::string path;
    Type type; // Can't const because of std::sort... Sacrifices must be made...
    TimeStamp lastModified;
  
    VirtualPath(const std::string &pathAsStr) : path(pathAsStr), type(std::filesystem::is_directory(pathAsStr) ? DICT : FILE), lastModified(last_modified(path)) {
      while (this->path.back() == '\\')
        this->path.pop_back();
    }

    inline size_t depth() const { // 3 (0 -> C:\, 1 -> Users, 2 -> txts, 3 -> exmpl.txt)
      sizeof(TimeStamp);
      size_t count = 0;
      for (char c : path)
        if (c == '\\')
          ++count;
      return count;
    }
  
    VirtualPath& operator=(const std::string &other) {
      path = other;
      if (path.back() == '\\')
        path.pop_back();
      type = std::filesystem::is_directory(other) ? DICT : FILE;
      lastModified = last_modified(path);
      return *this;
    }

    static TimeStamp last_modified(const std::string &filePath) { // Special thanks to co-pilot
      std::filesystem::file_time_type ftime = std::filesystem::last_write_time(filePath);
      std::chrono::system_clock::time_point timePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
      std::time_t cftime = std::chrono::system_clock::to_time_t(timePoint);
      struct tm* timeinfo = std::localtime(&cftime);
      return TimeStamp(timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900,
              timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }
  };



  class Folder { public:
    // Read only representation of a folder
    std::deque<VirtualPath> files;
    std::deque<Folder> folders;
    VirtualPath where;

    Folder(const std::string& location) : where(location) {
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(location)) {
        if (entry.is_directory())
          folders.push_back(Folder(entry.path().string()));
        else
          files.push_back(VirtualPath(entry.path().string()));
      }

      std::sort(files.begin(), files.end(), [](const VirtualPath& a, const VirtualPath& b) {
        return a.lastModified < b.lastModified;
      });
    }

    void update() {
      for (auto& file : files)
        file.lastModified = VirtualPath::last_modified(file.path);
      for (auto& folder : folders)
        folder.update();
      where.lastModified = VirtualPath::last_modified(where.path);
    }
  };
};