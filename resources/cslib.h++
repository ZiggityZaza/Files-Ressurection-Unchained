// LICENSE: ☝️🤓

// Including every single header that might ever be needed
#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <stop_token>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <iostream> // Already contains many libraries
#include <optional>
#include <utility>
#include <fstream>
#include <sstream>
#include <variant>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cctype>
#include <cstdio>
#include <random>
#include <vector>
#include <chrono>
#include <thread>
#include <locale>
#include <array>
#include <deque>
#include <cmath>
#include <list>
#include <map>
#include <set>
#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
#endif


#pragma once
/*
  Include this header file only once pwease. No support for
  linking and stuff.
  Prevent multiple inclusions of this header file
*/

#define CPLUSPLUSERRORMAKER_HELPER(x) #x
#define CPLUSPLUSERRORMAKER(x) CPLUSPLUSERRORMAKER_HELPER(x)
static_assert(__cplusplus >= 202002L, "Requires C++20 or newer. Current version is " CPLUSPLUSERRORMAKER(__cplusplus));




namespace cslib {
  // Jack of all trades (Helper functions and classes)

  // Other
  using csstr = const char *const; // C-style string
  using str_t = std::string;
  using strv_t = std::string_view;
  using byte_t = char;
  /*
    Differentiate between a char as byte and
    a char as character in a string
  */
  template <typename T>
  using cptr = const T *const;
  #define SHARED inline // Alias inline for shared functions, etc.
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO



  // Defined beforehand to avoid circular dependencies
  MACRO to_str(wchar_t _wchar) {
    if (_wchar > 0xFF) // If the character is not representable in a single byte
      throw std::runtime_error("to_str(wchar_t) cannot convert wide character to narrow string");
    return std::string(1, static_cast<char>(_wchar));
  }
  MACRO to_str(cptr<wchar_t> _cwstr) {
    // No constexpr version for std::wclen
    size_t len = 0;
    while (_cwstr[len] != 0)
      ++len;
    for (size_t i = 0; i < len; ++i)
      if (_cwstr[i] > 0xFF) // If the character is not representable in a single byte
        throw std::runtime_error("to_str(const wchar_t *const) cannot convert wide string to narrow string");
    return std::string(_cwstr, _cwstr + len);
  }
  MACRO to_str(const std::wstring& _wstr) {return std::string(_wstr.begin(), _wstr.end());}
  MACRO to_str(std::wstring_view _wstrv) {return std::string(_wstrv.begin(), _wstrv.end());}
  str_t to_str(const auto& _anything) {
    std::ostringstream oss;
    oss << _anything;
    return oss.str();
  }

  MACRO to_wstr(char _char) { return std::wstring(1, static_cast<wchar_t>(_char)); }
  MACRO to_wstr(csstr _cstr) {return std::wstring(_cstr, _cstr + std::strlen(_cstr));}
  MACRO to_wstr(const str_t& _str) {return std::wstring(_str.begin(), _str.end());}
  MACRO to_wstr(strv_t _strv) {return std::wstring(_strv.begin(), _strv.end());}
  std::wstring to_wstr(const auto& _anything) {
    std::wostringstream woss;
    woss << _anything;
    return woss.str();
  }



  MACRO wstrlen(std::wstring_view _wstrv) {
    return _wstrv.length();
  }
  MACRO strlen(std::string_view _strv) {
    return _strv.length();
  }



  class any_error : public std::runtime_error { public:
    /*
      Custom error class for cslib
      Example:
        throw cslib::any_error("Something went wrong");
    */
    any_error(size_t _lineInCode, const auto&... _msgs) : std::runtime_error([_lineInCode, &_msgs... ] {
      /*
        Create a custom error message with the given messages.
        Example:
          throw cslib::any_error(__LINE__, "Aye", L"yo", '!', 123, true);
      */
      std::ostringstream woss;
      woss << "cslib::any_error called in workspace " << std::filesystem::current_path();
      woss << " on line " << _lineInCode << " because: ";
      ((woss << to_str(std::forward<decltype(_msgs)>(_msgs))), ...);
      woss << std::flush;
      return woss.str();
    }()) {}
  };
  #define throw_up(...) throw cslib::any_error(__LINE__, __VA_ARGS__)



  void pause(size_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  void sh_call(strv_t _command) {
    /*
      Blocking system call
    */
    if (system(_command.data()) != 0)  
      throw_up("Failed to execute command: '", _command, "'");
  }



  MACRO contains(const auto& _lookIn, const auto& _lookFor) {
    /*
      does `container` contain `key`
    */
    return std::find(_lookIn.begin(), _lookIn.end(), _lookFor) != _lookIn.end();
  }
  MACRO have_something_common(const auto& _cont1, const auto& _cont2) {
    /*
      do `c1` and `c2` contain similar keys
    */
    for (const auto& item : _cont1)
      if (contains(_cont2, item))
        return true;
    return false;
  }



  MACRO recrusive_lookup(const auto& _lookIn, const auto& _lookFor) {
    /*
      Check if `container` contains `key` or if it contains
      a container that contains `key`
    */
    for (const auto& item : _lookIn) {
      if (item == _lookFor)
        return true;
      if constexpr (std::is_same_v<decltype(item), decltype(_lookIn)>)
        if (recrusive_lookup(item, _lookFor))
          return true;
    }
    return false;
  }



  str_t get_env(strv_t _var) {
    /*
      Get the value of an environment variable.
    */
    const char *const envCStr = getenv(_var.data());
    if (envCStr == NULL)
      throw_up("Environment variable '", _var, "' not found");
    return str_t(envCStr);
  }



  template <typename T>
  requires std::is_integral_v<T>
  std::vector<T> range(const T& _start, const T& _end) {
    /*
      Simplified range function that takes two integers
      and returns a vector of integers (inclusive)
    */
    std::vector<T> result;
    if (_start > _end) // reverse
      for (T i = _start; i > _end; --i)
        result.push_back(i);
    else if (_start < _end) // start to end
      for (T i = _start; i < _end; ++i)
        result.push_back(i);
    else // just start
      result.push_back(_start);
    return result;
  }
  template <typename T>
  requires std::is_integral_v<T>
  std::vector<T> range(const T& end) {
    return range(T(0), end);
  }



  auto retry(const auto& _func, size_t _maxAttempts, size_t _waitTimeMs) {
    /*
      Retry a function up to `retries` times with a delay
      of `delay` milliseconds between each retry.
      Example:
        std::function<int()> func = []() {
          // Do something that might fail
        };
        cslib::retry(func, 3, 0, 1);
    */
    while (_maxAttempts-- > 0) {
      try {
        static_assert(std::is_invocable_v<decltype(_func)>, "Function must be invocable");
        return _func();
      }
      catch (const std::exception& e) {
        if (_maxAttempts == 0)
          throw_up("Function failed after maximum attempts: ", e.what());
        std::this_thread::sleep_for(std::chrono::milliseconds(_waitTimeMs)); // Wait before retrying
      }
    }
    throw_up("Function failed after maximum attempts");
  }



  std::vector<strv_t> parse_cli_args(int _argc, csstr _args[]) {
    /*
      Parse command line arguments and return them as a
      vector of strings.
      Note:
        The first argument is the program name, so we skip it
    */
    if (_args == nullptr or _argc <= 0)
      throw_up("No command line arguments provided");
    std::vector<strv_t> args(_args, _args + _argc); // Includes binary name
    args.erase(args.begin()); // Remove the first argument (program name)
    return args;
  }



  str_t stringify_container(const auto& _vec) {
    /*
      Convert a vector to a string representation.
      Example:
        cslib::stringify_container({1, 2, 3}); // "{1, 2, 3}"
    */
    str_t result = "{";
    for (const auto& item : _vec)
      result += to_str(item) + ", ";
    if (result.length() > 1) { // If there are items
      result.pop_back(); // Remove the last comma
      result.pop_back(); // Remove the last space
    }
    result += "}";
    return result;
  }



  MACRO TRIM_WITH = "...";
  MACRO shorten_end(strv_t _strsv, size_t _maxLength) {
    /*
      Example:
        cslib::shorten_end(L"cslib.h++", 6); // "csl..."
    */
    if (_maxLength < strlen(TRIM_WITH))
      throw_up("maxLength must be at least ", strlen(TRIM_WITH), " (TRIM_WITH length)");
    if (_strsv.length() <= _maxLength)
      return str_t(_strsv);
    return str_t(_strsv.substr(0, _maxLength - strlen(TRIM_WITH))) + TRIM_WITH;
  }

  MACRO shorten_begin(strv_t _strsv, size_t _maxLength) {
    /*
      Example:
        cslib::shorten_begin(L"cslib.h++", 6); // "...h++"
    */
    if (_maxLength < strlen(TRIM_WITH))
      throw_up("maxLength must be at least ", strlen(TRIM_WITH), " (TRIM_WITH length)");
    if (_strsv.length() <= _maxLength)
      return str_t(_strsv);
    return str_t(TRIM_WITH) + str_t(_strsv.substr(_strsv.length() - (_maxLength - strlen(TRIM_WITH))));
  }



  std::vector<str_t> separate(strv_t _strsv, strv_t _delimiter) {
    /*
      Example:
        cslib::separate("John Money", " ) // {"John", "Money"}
    */
    std::vector<str_t> result;
    size_t pos = 0;
    while ((pos = _strsv.find(_delimiter)) != strv_t::npos) {
      result.push_back(str_t(_strsv.substr(0, pos)));
      _strsv.remove_prefix(pos + _delimiter.length());
    }
    if (!_strsv.empty())
      result.push_back(_strsv.data()); // Add the last part
    return result;
  }



  template <typename T>
  requires std::is_integral_v<T>
  T roll_dice(T _min, T _max) {
    /*
      Minimum and maximum value and returns a random
      number between them (inclusive).
      Example:
        cslib::roll_dice(1, 6); // Returns a random number
        between 1 and 6 (inclusive)
    */
    if (_min > _max) std::swap(_min, _max);
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<T> distribution(_min, _max);
    return distribution(generator);
  }



  str_t read_data(std::istream& _inStream) {
    /*
      Read all data from the given istream and return it as a string.
      After reading, the stream is considered empty.
      Throws an error if the stream is not open or in a bad state.
      Note:
        Handling encoding, other states, flags or similar are managed
        by the caller. This function only cleans up its own changes.
    */
    if (!_inStream or !_inStream.good())
      throw_up("std::istream is not good or in a bad state");
    std::streampos previousPos = _inStream.tellg();
    str_t result{std::istreambuf_iterator<char>(_inStream), std::istreambuf_iterator<char>()};
    _inStream.seekg(previousPos);
    return result;
  }
  void do_io(std::istream& _inStream, std::ostream& _outStream) {
      _outStream << read_data(_inStream) << std::flush;
    }



  class TimeStamp { public:
    /*
      A wrapper around std::chrono
    */
    std::chrono::system_clock::time_point timePoint;
    

    // Contructors and error handling
    TimeStamp() {timePoint = std::chrono::system_clock::now();}
    TimeStamp(std::chrono::system_clock::time_point _tp) : timePoint(_tp) {}
    TimeStamp(int _sec, int _min, int _hour, int _day, int _month, int _year) {
      /*
        Create a time stamp from the given date and time
        after making sure that the date is valid.
      */
      // Determine date
      std::chrono::year_month_day ymd{
        std::chrono::year(_year),
        std::chrono::month(_month),
        std::chrono::day(_day)
      };
      if (!ymd.ok())
        throw_up("Invalid date: ", _day, "-", _month, "-", _year);
      // Determine time
      if (_hour >= 24 or _min >= 60 or _sec >= 60)
        throw_up("Invalid time: ", _hour, ":", _min, ":", _sec);
      std::chrono::hh_mm_ss hms{
        std::chrono::hours(_hour) +
        std::chrono::minutes(_min) +
        std::chrono::seconds(_sec)
      };
      // Combine date and time into a time point
      timePoint = std::chrono::system_clock::time_point(
        std::chrono::sys_days(ymd).time_since_epoch() + hms.to_duration()
      );
    }


    str_t as_str() const {
      /*
        Convert the time point to (almost) ISO 8601
        in format HH:MM:SS DD-MM-YYYY)..
      */
      std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
      return (std::ostringstream() << std::put_time(std::gmtime(&time), "%H:%M:%S %d-%m-%Y")).str();
    }
    size_t year() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(int(ymd.year()));
    }
    size_t month() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(unsigned(ymd.month()));
    }
    size_t day() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(unsigned(ymd.day()));
    }
    size_t hour() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.hours().count());
    }
    size_t minute() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.minutes().count());
    }
    size_t second() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.seconds().count());
    }
  };



  MACRO Bold = "\033[1m";
  MACRO Underline = "\033[4m";
  MACRO Italic = "\033[3m";
  MACRO Reverse = "\033[7m";
  MACRO Hidden = "\033[8m";
  MACRO Black = "\033[30m";
  MACRO Red = "\033[31m";
  MACRO Green = "\033[32m";
  MACRO Yellow = "\033[33m";
  MACRO Blue = "\033[34m";
  MACRO Magenta = "\033[35m";
  MACRO Cyan = "\033[36m";
  MACRO White = "\033[37m";
  MACRO Reset = "\033[0m";
  class Out { public:
    /*
      Print to console with color and optionally into an existing file.
      Usage:
        cslib::Out error("Error: ", cslib::Out::Color::RED);
        error << "Something went wrong";
    */
    std::ostream& outTo;
    str_t prefix;
    Out() = default;
    Out(std::ostream& _outTo) : outTo(_outTo) {
      prefix = "";
    }
    Out(std::ostream& _outTo, strv_t _prefsv = "", strv_t _color = "") : outTo(_outTo) {
      std::ostringstream prefixStream;
      prefixStream << _color << _prefsv;
      if (!_prefsv.empty())
        prefixStream << " ";
      if (!_color.empty())
        prefixStream << Reset;
      prefix = prefixStream.str();
    }
    std::ostream& operator<<(const auto& _msg) const {
      return outTo << '[' << TimeStamp().as_str() << ']' << prefix << _msg;
    }
    std::ostream& operator<<(auto&& _msg) const {
      return outTo << '[' << TimeStamp().as_str() << ']' << prefix << std::forward<decltype(_msg)>(_msg);
    }
  };



  class Benchmark { public:
    /*
      Measures the time taken by a function or a block of code.
      Example:
        cslib::Benchmark benchmark;
        // Do something
        std::cout << "Time taken: " << benchmark.elapsed_ms() << " ms\n";
    */
    std::chrono::high_resolution_clock::time_point startTime;
    Benchmark() {
      startTime = std::chrono::high_resolution_clock::now();
    }
    void reset() {
      startTime = std::chrono::high_resolution_clock::now();
    }
    double elapsed_ns() {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    }
    double elapsed_ms() {
      return elapsed_ns() / 1'000'000.0f; // Convert nanoseconds to milliseconds
    }
    double elapsed_us() {
      return elapsed_ns() / 1'000.0f; // Convert nanoseconds to microseconds
    }
  };



  class Road { public:
    /*
      Abstract base class for filesystem entries.
      Represents a path in the filesystem and provides
      methods to manage it.
    */
    std::filesystem::path isAt; // Covers move and copy semantics


    // Path management
    std::wstring wstr() const { return isAt.wstring(); }
    std::string str() const { return isAt.string(); }
    std::string name() const { return isAt.filename().string(); }
    std::filesystem::file_type type() const {
      return std::filesystem::status(isAt).type();
    }
    std::chrono::system_clock::time_point last_modified() const { 
      auto ftime = std::filesystem::last_write_time(isAt);
      return std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    }
    Road parent() const {
      return Road(isAt.parent_path(), std::filesystem::file_type::directory);
    }
    size_t depth() const {
      /*
        Example:
          Road path("/gitstuff/cslib/cslib.h++");
          size_t depth = path.depth();
          // depth = 3 (because there are 3 directories before the file)
      */
      return separate(isAt.string(), to_str(std::filesystem::path::preferred_separator)).size() - 1;
    }
    void rename_self_to(strv_t _newName) {
      /*
        Rename the entry to a new name.
        Note:
          The new name must not contain any path separators.
      */
      if (_newName.find(std::filesystem::path::preferred_separator) != std::wstring::npos)
        throw_up("New name '", _newName, "' contains path separators");
      std::filesystem::path newPath = isAt.parent_path() / _newName;
      if (std::filesystem::exists(newPath))
        throw_up("Path ", newPath, " already exists");
      std::filesystem::rename(isAt, newPath);
      isAt = newPath; // Update the path
    }


    bool operator==(const Road& _other) const { return this->isAt == _other.isAt; }
    bool operator!=(const Road& _other) const { return !(*this == _other); }
    bool operator==(const std::filesystem::path& _other) const { return this->isAt == _other; }
    bool operator!=(const std::filesystem::path& _other) const { return !(*this == _other); }
    // Implicitly converts wide (c-)strings to std::filesystem::path 


    // Transform into stl
    operator std::string() const { return this->str(); }
    operator std::filesystem::path() const { return this->isAt; }
    operator std::filesystem::path&() { return this->isAt; }
    operator const std::filesystem::path&() const { return this->isAt; }
    operator std::filesystem::path*() { return &this->isAt; }
    operator std::filesystem::path*() const { return const_cast<std::filesystem::path*>(&this->isAt); } // casted immutable
    friend std::ostream& operator<<(std::ostream& _out, const Road& _entry) {
      return _out << _entry.str();
    }


    Road() = default;
    Road(const std::filesystem::path& _where) : isAt(std::filesystem::canonical(_where)) {}
    Road(const std::filesystem::path& _where, std::filesystem::file_type _type) : isAt(_where) {
    if (_type != this->type())
      throw_up("Road ", _where, " initialized with unexpected file type");
    }
  };



  class Folder : public Road { public:
    /*
      Child class of Path that represents a folder.
      Example:
        Folder folder("/gitstuff/cslib");
        if (folder.has(Road("/gitstuff/cslib/cslib.h++")))
          std::wcout << "Folder contains the file\n";
    */

    Folder() = default;
    Folder(const std::filesystem::path& _where, bool _createIfNotExists = false) {
      if (_where.empty())
        throw_up("Path empty");
      if (_createIfNotExists and !std::filesystem::exists(_where)) {
        std::filesystem::create_directory(_where);
        if (!std::filesystem::exists(_where))
          throw_up("Failed to create folder ", _where);
      } else if (!std::filesystem::is_directory(_where))
        throw_up("Path ", _where, " is not a directory");
      isAt = std::filesystem::canonical(_where);
    }


    std::vector<Road> list() const {
      /*
        List all entries in the folder.
        Returns a vector of Path, File and Folder objects.
      */
      std::vector<Road> entries;
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(isAt))
        entries.emplace_back(Road(entry.path()));
      return entries;
    }
    bool has(const Road& _item) const {
      return contains(list(), _item);
    }


    void move_self_into(Folder& _newLocation) {
      /*
        Important note:
          Upon moving, subfolders and files objects
          will still point to the old location.
      */
      if (std::filesystem::exists(_newLocation.isAt / isAt.filename()))
        throw_up("Path ", isAt, " already exists in folder ", _newLocation.isAt);
      std::filesystem::rename(isAt, _newLocation.isAt / isAt.filename());
      isAt = _newLocation.isAt / isAt.filename();
    }


    Folder copy_self_into(Folder& _newLocation, std::filesystem::copy_options _options = std::filesystem::copy_options::recursive) const {
      std::filesystem::copy(isAt, _newLocation.isAt / isAt.filename(), _options);
      return Folder(_newLocation.isAt / isAt.filename());
    }
  };



  class File : public Road { public:
    /*
      Child class of RouteToFile that represents a file.
      Example:
        File file("/gitstuff/cslib/cslib.h++");
        str_t content = file.content();
        // content = "Around 50 years ago, a group of people..."
    */

    File() = default;
    File(const std::filesystem::path& _where, bool _createIfNotExists = false) {
      if (_where.empty())
        throw_up("Path empty");
      if (_createIfNotExists and !std::filesystem::exists(_where))
        std::ofstream file(_where);
      if (!std::filesystem::is_regular_file(_where))
        throw_up("Path ", _where, " is not a regular file");
      isAt = std::filesystem::canonical(_where);
    }


    std::ifstream reach_in(std::ios::openmode mode) const {
      std::ifstream file(isAt, mode);
      if (!file.is_open() or !file.good())
        throw_up("Failed to open file ", isAt);
      return std::move(file);
    }
    std::ofstream reach_out(std::ios::openmode mode) const {
      std::ofstream file(isAt, mode);
      if (!file.is_open() or !file.good())
        throw_up("Failed to open file ", isAt);
      return std::move(file);
    }

    str_t read_text() const {
      std::ifstream file(reach_in(std::ios::in));
      return str_t((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    void edit_text(const auto& content) const {
      reach_out(std::ios::out | std::ios::trunc) << content;
    }
    void add_text(const auto& content) const {
      reach_out(std::ios::out | std::ios::app) << content;
    }


    std::vector<byte_t> read_data() const {
      /*
        Streambufs for byte-by-byte reading
        for raw data reading.
      */
      std::ifstream file(reach_in(std::ios::binary));
      return std::vector<byte_t> {
        std::istreambuf_iterator<byte_t>(file),
        std::istreambuf_iterator<byte_t>()
      };
    }
    void edit_binary_data(const std::vector<char>& _data) const {
      std::ofstream file(reach_out(std::ios::binary | std::ios::trunc));
      file.write(_data.data(), _data.size());
      if (!file.good())
        throw_up("Failed to write into file ", isAt);
    }
    void add_binary_data(const std::vector<char>& _data) const {
      std::ofstream file(reach_out(std::ios::binary | std::ios::app));
      file.write(_data.data(), _data.size());
      if (!file.good())
        throw_up("Failed to write into file ", isAt);
    }

    std::wstring extension() const {return isAt.extension().wstring();}
    size_t bytes() const {return std::filesystem::file_size(isAt);}


    void move_self_into(Folder& _newLocation) {
      if (std::filesystem::exists(_newLocation.isAt / isAt.filename()))
        throw_up("Path ", isAt, " already exists in folder ", _newLocation.isAt);
      std::filesystem::rename(isAt, _newLocation.isAt / isAt.filename());
      isAt = _newLocation.isAt / isAt.filename(); // Update the path
    }


    File copy_self_into(Folder& _newLocation, std::filesystem::copy_options _options = std::filesystem::copy_options::none) const {
      std::filesystem::copy(isAt, _newLocation.isAt / isAt.filename(), _options);
      return File(_newLocation.isAt / isAt.filename());
    }
  };



  std::vector<std::variant<File, Folder, Road>> deduct_list_items(const std::vector<Road>& _list) {
    /*
      Deduct the types of paths in the list.
      Note:
        Defined after the class declarations
        to ensure all types are known.
    */
    std::vector<std::variant<File, Folder, Road>> result;
    for (const Road& entry : _list)
      if (entry.type() == std::filesystem::file_type::regular)
        result.emplace_back(File(entry));
      else if (entry.type() == std::filesystem::file_type::directory)
        result.emplace_back(Folder(entry));
      else
        result.emplace_back(entry); // Other types like symlinks
    return result;
  }



  MACRO RANDOM_ENTRY_NAME_LEN = 2; // n^59 possible combinations
  str_t scramble_filename() {
    /*
      Generate a random filename with a length of `RANDOM_ENTRY_NAME_LEN`
      characters. The filename consists of uppercase and lowercase
      letters and digits.
    */
    std::ostringstream randomName;
    for ([[maybe_unused]] auto _ : range(RANDOM_ENTRY_NAME_LEN))
      switch (roll_dice(0, 2)) {
        case 0: randomName << roll_dice('A', 'Z'); break; // Uppercase letter
        case 1: randomName << roll_dice('a', 'z'); break; // Lowercase letter
        case 2: randomName << roll_dice('0', '9'); break; // Digit
        default: randomName << '?';
      }
    return randomName.str();
  }



  class TempFile : public File { public:
    /*
      Create a temporary file with a random name in the system's
      temporary directory. The name is generated by rolling dice
      to create a random string of letters and digits.
    */
    TempFile() {
      str_t tempFileName = "cslibTempFile_" + scramble_filename() + ".tmp";
      // Ensure the file does not already exist
      do {
        tempFileName = "cslibTempFile_" + scramble_filename() + ".tmp";
      } while (std::filesystem::exists(std::filesystem::temp_directory_path() / tempFileName));

      isAt = File(std::filesystem::temp_directory_path() / tempFileName, true).isAt; // Create the file
    }
    TempFile(const std::filesystem::path& _createAs) {
      if (std::filesystem::exists(_createAs))
        throw_up("File ", _createAs.filename(), " already exists in folder ", _createAs.parent_path());
      isAt = File(_createAs, true).isAt; // Create the file
    }
    ~TempFile() {
      if (std::filesystem::exists(isAt))
        std::filesystem::remove(isAt);
    }

    TempFile(const TempFile& _other) {
      /*
        Create a copy of a file with the
        same content but different name.
        Note:
          The original file will not be deleted.
      */
      isAt = _other.isAt; // Copy the path
      str_t newName;
      do {
        newName = "cslibTempFile_" + scramble_filename() + ".tmp";
      } while (std::filesystem::exists(std::filesystem::temp_directory_path() / newName));
      isAt = File(std::filesystem::temp_directory_path() / newName, true).isAt; // Ensure the file does not already exist
      // Copy the content
    }
  };

  TempFile& make_runtime_file() {
    /*
      Return a reference to a temporary file that is created
      at runtime. The file is created in a static deque to
      ensure that it is not deleted until the program exits.
      Note:
        Using deque cuz vector reallocation causes the
        file to be deleted and/or its memory address to
        change, which would lead to undefined behavior.
    */
    static std::deque<TempFile> runtimeFiles;
    runtimeFiles.emplace_back();
    return runtimeFiles.back();
  }



  class TempFolder : public Folder { public:
    /*
      Create a temporary folder with a random name in the system's
      temporary directory. The name is generated by rolling dice
      to create a random string of letters and digits.
      Note:
        Destructor also takes all files in the folder
        into account and deletes them.
    */
    TempFolder() {
      str_t tempFolderName;
      do {
        tempFolderName = "cslibTempFolder_" + scramble_filename();
      } while (std::filesystem::exists(std::filesystem::temp_directory_path() / tempFolderName));
      isAt = Folder(std::filesystem::temp_directory_path() / tempFolderName, true).isAt;
    }
    TempFolder(const std::filesystem::path& _createAs) {
      /*
        Create a temporary folder in the given path.
        Note:
          If the folder already exists, it will throw an error.
      */
      if (std::filesystem::exists(_createAs))
        throw_up("Folder ", _createAs.filename(), " already exists in folder ", _createAs.parent_path());
      isAt = Folder(_createAs, true).isAt;
    }
    ~TempFolder() {
      if (std::filesystem::exists(isAt))
        std::filesystem::remove_all(isAt);
    }

    TempFolder(const TempFolder&) = delete;
    TempFolder& operator=(const TempFolder&) = delete;
  };

  Folder& make_runtime_folder() {
    /*
      Return a reference to a temporary folder that is created
      at runtime. The folder is created in a static deque to
      ensure that it is not deleted until the program exits.
      Note:
        Same as make_runtime_file, using deque to avoid
        reallocation issues.
    */
    static std::deque<TempFolder> runtimeFolders;
    runtimeFolders.emplace_back();
    return runtimeFolders.back();
  }



  MACRO CONFIG_FILE_NAME = "cslib_configs.ini";
  /*
    Configuration file for cslib. The file is used
    to store configuration options for cslib and
    other libraries that use cslib.
  */
  File find_configsFile(bool _createIfNotExists) {
    /*
      Get the cslib configs file in the application data folder.
      If the file does not exist, it will be created.
      Returns a File object.
    */
    Folder appDataFolder = [] {
      /*
        Find the application data folder for the current user.
        Returns a Folder object pointing to the folder.
        Note:
          If the folder does not exist, it will be created.
      */
      std::filesystem::path appDataPath;
      #ifdef _WIN32
        appDataPath = std::filesystem::path(get_env("APPDATA"));
      #else
        try {
          appDataPath = std::filesystem::path(get_env("XDG_CONFIG_HOME"));
        } catch (cslib::any_error&) {
          appDataPath = std::filesystem::path(get_env("HOME")) / ".config";
        }
      #endif
      return Folder(appDataPath, false); // Don't create if not exists
    }();
    return File(appDataFolder.isAt / CONFIG_FILE_NAME, _createIfNotExists); // Create if not exists
  }



  std::map<str_t, std::map<str_t, str_t>> parse_configsFile(const File& _configsFile) {
    std::map<str_t, std::map<str_t, str_t>> configs;
    str_t content = _configsFile.read_text();
    str_t currentSection;
    for (strv_t line : separate(content, "\n")) {
      str_t trimmedLine(line);
      trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t")); // Remove leading whitespace
      trimmedLine.erase(trimmedLine.find_last_not_of(" \t") + 1); // Remove trailing whitespace
      if (trimmedLine.empty() or trimmedLine.at(0) == ';')
        continue; // Skip empty lines and comments
      if (trimmedLine.at(0) == '[' and trimmedLine.back() == ']') {
        // Section header
        currentSection = trimmedLine.substr(1, trimmedLine.length() - 2);
        configs.insert({currentSection, {}});
      } else {
        // Key-value pair
        size_t eqPos = trimmedLine.find('=');
        if (eqPos == str_t::npos)
          throw_up("Invalid line in config file: '", line, "'");
        str_t key = trimmedLine.substr(0, eqPos);
        str_t value = trimmedLine.substr(eqPos + 1);
        key.erase(0, key.find_first_not_of(" \t")); // Remove leading whitespace
        key.erase(key.find_last_not_of(" \t") + 1); // Remove trailing whitespace
        value.erase(0, value.find_first_not_of(" \t")); // Remove leading whitespace
        value.erase(value.find_last_not_of(" \t") + 1); // Remove trailing whitespace
        if (currentSection.empty())
          throw_up("Key-value pair without section in config file: '", line, "'");
        if (configs.find(currentSection) == configs.end())
          throw_up("Key-value pair in unknown section in config file: '", line, "'");
        configs[currentSection].insert({key, value});
      }
    }
    return configs;
  }
} // namespace cslib