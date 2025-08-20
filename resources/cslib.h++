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
  using cstr = const char *const; // C-style string
  using str_t = std::string;
  using strv_t = std::string_view;
  using byte_t = char;
  /*
    Differentiate between a char as byte and
    a char as character in a string
  */
  template <typename T>
  using cptr = const T *const;
  template <typename T>
  using maybe = std::optional<T>;
  #define SHARED inline // Alias inline for shared functions, etc.
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO



  // Defined beforehand to avoid circular dependencies
  MACRO to_str(wchar_t _wchar) {
    if (_wchar > 0xFF) // If the character is not representable in a single byte
      throw std::runtime_error("Can't convert wide character to narrow ones");
    return std::string(1, static_cast<char>(_wchar));
  }
  MACRO to_str(cptr<wchar_t> _cwstr) {
    // No constexpr version for std::wclen
    size_t len = 0;
    while (_cwstr[len] != 0)
      ++len;
    for (size_t i = 0; i < len; ++i)
      if (_cwstr[i] > 0xFF) // If the character is not representable in a single byte
        throw std::runtime_error("Can't convert wide string to narrow string");
    return std::string(_cwstr, _cwstr + len);
  }
  MACRO to_str(std::wstring_view _wstrv) {return to_str(_wstrv.data());}
  MACRO to_str(const std::wstring& _wstr) { return to_str(_wstr.data()); }
  str_t to_str(const auto& _anything) {
    std::ostringstream oss;
    oss << _anything;
    return oss.str();
  }

  MACRO to_wstr(char _char) { return std::wstring(1, static_cast<wchar_t>(_char)); }
  MACRO to_wstr(cstr _cstr) {return std::wstring(_cstr, _cstr + std::strlen(_cstr));}
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
  #define cslib_throw_up(...) throw cslib::any_error(__LINE__, __VA_ARGS__)



  void pause(size_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  str_t sh_call(strv_t _command) {
    /*
      Non-blocking system call that returns the
      output of the command. Throws if the command
      does not exist or fails to execute.
      Important Note:
        This method is VERY prone to injections
    */
    std::array<char, 128> buffer = {};
    str_t result;
    int exitCode = 0;
    FILE* pipe = popen(_command.data(), "r");
    if (!pipe)
      cslib_throw_up("Failed to open pipe");
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
      result += buffer.data();
    exitCode = pclose(pipe);
    if (exitCode != 0)
      cslib_throw_up("Command failed or not found: '", _command, "' (exit code ", exitCode, ")");
    return result;
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
    cstr envCStr = getenv(_var.data());
    if (envCStr == NULL)
      cslib_throw_up("Environment variable '", _var, "' not found");
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
  template <typename T>
  std::vector<T> range(cptr<T> _begin, size_t _count) {
    return std::vector<T>(_begin, _begin + _count);
  }



  auto retry(const auto& _func, size_t _maxAttempts, size_t _waitTimeMs) {
    /*
      Retry a function up to `retries` times with a delay
      of `delay` milliseconds between each retry.
      Example:
        std::function<int()> func = [] {
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
          cslib_throw_up("Function failed after maximum attempts: ", e.what());
        std::this_thread::sleep_for(std::chrono::milliseconds(_waitTimeMs)); // Wait before retrying
      }
    }
    cslib_throw_up("Function failed after maximum attempts");
  }



  std::vector<strv_t> parse_cli_args(int _argc, cstr _args[]) {
    /*
      Parse command line arguments and return them as a
      vector of strings.
      Note:
        The first argument is the program name, so we skip it
    */
    if (_args == nullptr or _argc <= 0)
      cslib_throw_up("No command line arguments provided");
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
      cslib_throw_up("maxLength must be at least ", strlen(TRIM_WITH), " (TRIM_WITH length)");
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
      cslib_throw_up("maxLength must be at least ", strlen(TRIM_WITH), " (TRIM_WITH length)");
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
      cslib_throw_up("std::istream is not good or in a bad state");
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
        cslib_throw_up("Invalid date: ", _day, "-", _month, "-", _year);
      // Determine time
      if (_hour >= 24 or _min >= 60 or _sec >= 60)
        cslib_throw_up("Invalid time: ", _hour, ":", _min, ":", _sec);
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
      if (_newName.find(std::filesystem::path::preferred_separator) != std::string::npos)
        cslib_throw_up("New name '", _newName, "' contains path separators");
      std::filesystem::path newPath = isAt.parent_path() / _newName;
      if (std::filesystem::exists(newPath))
        cslib_throw_up("Path ", newPath, " already exists");
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
      cslib_throw_up("Road ", _where, " initialized with unexpected file type");
    }
  };




  class File;
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
        cslib_throw_up("Path empty");
      if (_createIfNotExists and !std::filesystem::exists(_where)) {
        std::filesystem::create_directory(_where);
        if (!std::filesystem::exists(_where))
          cslib_throw_up("Failed to create folder ", _where);
      } else if (!std::filesystem::is_directory(_where))
        cslib_throw_up("Path ", _where, " is not a directory");
      isAt = std::filesystem::canonical(_where);    
    }
    Folder(const Road& _where, bool _createIfNotExists) : Folder(_where.isAt, _createIfNotExists) {}


    std::vector<Road> list() const {
      /*
        List all entries in the folder.
        Returns a vector of Path, File and Folder objects.
        Note:
          Sorted by name
      */
      std::vector<Road> entries;
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(isAt))
        entries.emplace_back(entry.path());
      std::sort(entries.begin(), entries.end(), [](const Road& a, const Road& b) {
        return a.str() < b.str();
      });
      return entries;
    }
    std::vector<std::variant<File, Folder, Road>> typed_list() const;
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
        cslib_throw_up("Path ", isAt, " already exists in folder ", _newLocation.isAt);
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
        cslib_throw_up("Path empty");
      if (_createIfNotExists and !std::filesystem::exists(_where))
        std::ofstream file(_where);
      if (!std::filesystem::is_regular_file(_where))
        cslib_throw_up("Path ", _where, " is not a regular file");
      isAt = std::filesystem::canonical(_where);
    }
    File(const Road& _where, bool _createIfNotExists) : File(_where.isAt, _createIfNotExists) {}


    std::ifstream reach_in(std::ios::openmode mode) const {
      std::ifstream file(isAt, mode);
      if (!file.is_open() or !file.good())
        cslib_throw_up("Failed to open file ", isAt);
      return std::move(file);
    }
    std::ofstream reach_out(std::ios::openmode mode) const {
      std::ofstream file(isAt, mode);
      if (!file.is_open() or !file.good())
        cslib_throw_up("Failed to open file ", isAt);
      return std::move(file);
    }

    str_t read_text() const {
      std::ifstream file(reach_in(std::ios::in));
      return str_t((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    void edit_text(const auto& _newText) const {
      reach_out(std::ios::out) << _newText;
    }


    std::vector<byte_t> read_binary() const {
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
    void edit_binary(const auto *const _newData, size_t _len) const {
      std::ofstream file(reach_out(std::ios::binary | std::ios::trunc));
      file.write(reinterpret_cast<const char *const>(_newData), _len);
      if (!file.good())
        cslib_throw_up("Failed to write into file ", isAt);
    }

    std::string extension() const {return isAt.extension().string();}
    size_t bytes() const {return std::filesystem::file_size(isAt);}


    void move_self_into(Folder& _newLocation) {
      if (std::filesystem::exists(_newLocation / isAt.filename()))
        cslib_throw_up("Path ", isAt, " already exists in folder ", _newLocation.isAt);
      std::filesystem::rename(isAt, _newLocation / isAt.filename());
      isAt = _newLocation / isAt.filename(); // Update the path
    }


    File copy_self_into(Folder& _newLocation, std::filesystem::copy_options _options = std::filesystem::copy_options::none) const {
      std::filesystem::copy(isAt, _newLocation / isAt.filename(), _options);
      return File(_newLocation / isAt.filename());
    }
  };



  std::vector<std::variant<File, Folder, Road>> Folder::typed_list() const {
    std::vector<std::variant<File, Folder, Road>> result;
    for (const Road& entry : this->list())
      if (entry.type() == std::filesystem::file_type::regular)
        result.emplace_back(File(entry));
      else if (entry.type() == std::filesystem::file_type::directory)
        result.emplace_back(Folder(entry));
      else
        result.emplace_back(entry); // Other types like symlinks
    return result;
  }



  MACRO SCRAMBLE_LEN = 2; // n^59 possible combinations hehe
  str_t scramble_filename() {
    /*
      Generate a random filename with a length of `SCRAMBLE_LEN`
      characters. The filename consists of uppercase and lowercase
      letters and digits.
    */
    std::ostringstream randomName;
    for ([[maybe_unused]] auto _ : range(SCRAMBLE_LEN))
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
      str_t tempFileName;
      // Ensure the file does not already exist
      do {
        tempFileName = "cslibTempFile_" + scramble_filename() + ".tmp";
      } while (std::filesystem::exists(std::filesystem::temp_directory_path() / tempFileName));

      isAt = File(std::filesystem::temp_directory_path() / tempFileName, true).isAt; // Create the file
    }
    TempFile(const std::filesystem::path& _createAs) {
      if (std::filesystem::exists(_createAs))
        cslib_throw_up("File ", _createAs.filename(), " already exists in folder ", _createAs.parent_path());
      isAt = File(_createAs, true).isAt; // Create the file
    }
    ~TempFile() {
      if (std::filesystem::exists(isAt))
        std::filesystem::remove(isAt);
    }

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
  };



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
        cslib_throw_up("Folder ", _createAs.filename(), " already exists in folder ", _createAs.parent_path());
      isAt = Folder(_createAs, true).isAt;
    }
    ~TempFolder() {
      if (std::filesystem::exists(isAt))
        std::filesystem::remove_all(isAt);
    }

    TempFolder(const TempFolder&) = delete;
    TempFolder& operator=(const TempFolder&) = delete;
  };



  str_t wget(strv_t _url) {
    /*
      Download the content of the given URL using wget
      and return it as a string.
      Note:
        This function requires wget to be installed on the system.
        It will throw an error if wget is not found or fails to execute.
    */
    return sh_call("wget -q -O - " + str_t(_url.data())); // -q = quiet mode, -O - = output to stdout
  }




  template <typename T>
  requires std::is_arithmetic_v<T>
  MACRO highest_value_of() {
    /*
      Get the highest possible value that
      the type T can represent.
    */
    if constexpr (std::is_integral_v<T>)
      return std::numeric_limits<T>::max();
    else if constexpr (std::is_floating_point_v<T>)
      return std::numeric_limits<T>::infinity();
    else
      cslib_throw_up("Unsupported type for highest_value_of");
  }



  template <typename T>
  requires std::is_arithmetic_v<T>
  MACRO lowest_value_of() {
    /*
      Get the lowest possible value that
      the type T can represent.
    */
    if constexpr (std::is_integral_v<T>)
      return std::numeric_limits<T>::lowest();
    else if constexpr (std::is_floating_point_v<T>)
      return -std::numeric_limits<T>::infinity();
    else
      cslib_throw_up("Unsupported type for lowest_value_of");
  }



  template <typename T>
  FIXED T& grab_var(std::optional<T>& _optional) {
    return _optional.value();
  }
  template <typename T>
  FIXED T& grab_var(const auto& _variant) {
    if (!std::holds_alternative<T>(_variant))
      throw_up("Expected variant type ", typeid(T).name(), " but got ", _variant.index());
    return std::get<T>(_variant);
  }
} // namespace cslib