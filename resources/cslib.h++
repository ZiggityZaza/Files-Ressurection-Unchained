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
#include <fstream>
#include <variant>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cctype>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>
#include <chrono>
#include <thread>
#include <array>
#include <deque>
#include <cmath>
#include <list>
#include <map>
#include <set>


#pragma once
/*
  Include this header file only once pwease. No support for
  linking and stuff.
  Prevent multiple inclusions of this header file
*/


#if __cplusplus < 202002L
  #error "Requires C++ >= 20"
#endif




namespace cslib {
  // Jack of all trades (Helper functions and classes)

  // Other
  using wstr_t = std::wstring;
  using wstrsv_t = std::wstring_view;
  #define SHARED inline // Alias inline for shared functions, etc.
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO



  // Functions
  void pause(size_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  // Find the correct way to convert T to string
  template <typename T>
  std::string to_ptrstr(T* value) {
    /*
      Convert a pointer to a cool looking
      pointer address string.
      Example:
        cslib::to_str(&value) // "0x123"
    */
    if (value == nullptr)
      return "nullptr"; // Handle null pointers
    std::ostringstream oss;
    oss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(value);
    return oss.str();
  }
  std::string to_str(char value) {
    return std::string(1, value); // Convert char to string
  }
  std::string to_str(wchar_t value) {
    return std::string(1, static_cast<char>(value)); // Convert wchar_t to string
  }
  std::string to_str(std::string_view value) {
    /*
      Also for const char*
    */
    return std::string(value.data(), value.data() + value.size());
  }
  std::string to_str(wstrsv_t value) {
    /*
      Also for const wchar_t*
      Note: This is a lossy conversion, as wchar_t can represent characters
      that cannot be represented in a single byte string.
      Use with caution, as it may lead to data loss.
    */
    return std::string(value.data(), value.data() + value.size());
  }
  template <std::integral T>
  std::string to_str(T value) {
    return std::string(std::to_string(value));
  }
  template <std::floating_point T>
  std::string to_str(T value) {
    return std::string(std::to_string(value));
  }

  // Find the correct way to convert T to wide string
  template <typename T>
  wstr_t to_ptrwstr(T* value) {
    /*
      Convert a pointer to a cool looking
      pointer address string.
      Example:
        cslib::to_wstr(&value) // "0x123"
    */
    if (value == nullptr)
      return L"nullptr"; // Handle null pointers
    std::wostringstream oss;
    oss << L"0x" << std::hex << reinterpret_cast<std::uintptr_t>(value);
    return oss.str();
  }
  wstr_t to_wstr(char value) {
    return wstr_t(1, static_cast<wchar_t>(value));
  }
  wstr_t to_wstr(wchar_t value) {
    return wstr_t(1, value);
  }
  wstr_t to_wstr(std::string_view value) {
    /*
      Also for const char*
    */
    return wstr_t(value.data(), value.data() + value.size());
  }
  wstr_t to_wstr(wstrsv_t value) {
    /*
      Also for const wchar_t*
    */
    return wstr_t(value.data(), value.data() + value.size());
  }
  template <std::integral T>
  wstr_t to_wstr(T value) {
    return wstr_t(std::to_wstring(value));
  }
  template <std::floating_point T>
  wstr_t to_wstr(T value) {
    return wstr_t(std::to_wstring(value));
  }



  template <typename T>
  void print(const T& msg) {
    std::wcout << msg << std::flush;
  }
  template <typename T>
  void print(T&& msg) {
    std::wcout << std::forward<T>(msg) << std::flush;
  }
  void print(const char *const msg) {
    std::wcout << to_wstr(msg) << std::flush;
  }
  void print(const std::string& msg) {
    std::wcout << to_wstr(msg) << std::flush;
  }
  void print(std::string&& msg) {
    std::wcout << to_wstr(std::move(msg)) << std::flush;
  }
  void print(std::string_view msg) {
    std::wcout << to_wstr(msg) << std::flush;
  }

  template <typename T>
  void println(const T& msg) {
    std::wcout << msg << std::endl;
  }
  template <typename T>
  void println(T&& msg) {
    std::wcout << std::forward<T>(msg) << std::endl;
  }
  void println(const char *const msg) {
    std::wcout << to_wstr(msg) << std::endl;
  }
  void println(const std::string& msg) {
    std::wcout << to_wstr(msg) << std::endl;
  }
  void println(std::string&& msg) {
    std::wcout << to_wstr(std::move(msg)) << std::endl;
  }
  void println(std::string_view msg) {
    std::wcout << to_wstr(msg) << std::endl;
  }



  template <typename... Args>
  std::runtime_error up_impl(size_t line, Args&&... messages) {
    /*
      Create a custom runtime error with the given messages.
      Example:
        #define up(...) up_impl(__LINE__, __VA_ARGS__)
        if (1 == 2)
          throw up("Hello", "World", 123, L"Wide string");
    */
    wstr_t message;
    ((message += to_wstr(std::forward<Args>(messages))), ...);
    std::wcout << L"\033[1m" << L"\033[31m" << L"Error: " << message << L"\033[0m" << std::endl;
    std::filesystem::path currentPath = std::filesystem::current_path();
    return std::runtime_error("std::runtime_error called from line " + std::to_string(line) + " in workspace '" + currentPath.string() + "'");
  }
  #define throw_up(...) throw up_impl(__LINE__, __VA_ARGS__)



  void sh_call(std::string_view command) {
    /*
      Blocking system call
    */
    if (system(command.data()) != 0)  
      throw_up("Failed to execute command: ", command);
  }



  void clear_console() {
    #ifdef _WIN32
      sh_call("cls");
    #else
      sh_call("clear");
    #endif
  }



  template <typename Key, typename Container>
  bool contains(Container& lookIn, Key& lookFor) {
    /*
      does `container` contain `key`
    */
    return std::find(lookIn.begin(), lookIn.end(), lookFor) != lookIn.end();
  }
  template <typename Containers>
  bool have_something_common(Containers& c1, Containers& c2) {
    /*
      do `c1` and `c2` contain similar keys
    */
    for (auto item : c1)
      if (contains(c2, item))
        return true;
    return false;
  }



  std::string get_env(std::string_view var) {
    /*
      Get the value of an environment variable.
    */
    const char *const envCStr = getenv(var.data());
    if (envCStr == NULL)
      throw_up("Environment variable '", var, "' not found");
    return std::string(envCStr);
  }



  std::vector<int> range(int start, int end) {
    /*
      Simplified range function that takes two integers
      and returns a vector of integers (inclusive)
    */
    std::vector<int> result;
    if (start > end) // reverse
      for (int i = start; i >= end; --i)
        result.push_back(i);
    else if (start < end) // start to end
      for (int i = start; i <= end; ++i)
        result.push_back(i);
    else // just start
      result.push_back(start);
    return result;
  }
  std::vector<int> range(int end) {
    return range(0, end);
  }
  std::vector<size_t> range(size_t start, size_t end) {
    std::vector<size_t> result;
    if (start > end) // reverse
      for (size_t i = start; i >= end; --i)
        result.push_back(i);
    else if (start < end) // start to end
      for (size_t i = start; i <= end; ++i)
        result.push_back(i);
    else // just start
      result.push_back(start);
    return result;
  }
  std::vector<size_t> range(size_t end) {
    return range(size_t(0), end);
  }



  template <typename T>
  T retry(const std::function<T()>& target, size_t retries, size_t delay = 0) {
    /*
      Retry a function up to `retries` times with a delay
      of `delay` milliseconds between each retry.
      Note:
        No lambda support
      Example:
        std::function<void()> func = []() {
          // Do something that might fail
        };
        cslib::retry(func, 3);
    */
    if (retries == 0)
      throw_up("Retries must be greater than 0");
    for (size_t tried : range(retries)) {
      try {
        return target();
      } catch (const std::exception& e) {
        if (tried == retries - 1) {
          throw_up("Function ", to_ptrstr(&target), " failed after ", retries, " retries: ", e.what());
        }
      } catch (...) {
        // Catch all other exceptions
        if (tried == retries - 1) {
          throw_up("Function ", to_ptrstr(&target), " failed after ", retries, " retries: Unknown exception");
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
    return T(); // This line is unreachable but keeps compiler happy
  }



  std::vector<std::string> parse_cli_args(int argc, const char *const argv[]) {
    /*
      Parse command line arguments and return them as a
      vector of strings.
      Note:
        The first argument is the program name, so we skip it
    */
    std::vector<std::string> args;
    if (argc <= 1)
      return args; // No arguments provided
    for (int i : range(1, argc - 1))
      args.emplace_back(argv[i]);
    return args;
  }



  FIXED wstrsv_t TRIM_WITH = L"...";
  wstr_t shorten_end(wstrsv_t wstrsv, size_t maxLength) {
    /*
      Trim and content of `TRIM_WITH` to the end of the string
      if it exceeds `maxLength`.
      Example:
        cslib::shorten_end(L"cslib.h++", 3);
        // is "c..."
    */
    if (maxLength < TRIM_WITH.length())
      throw_up("maxLength must be greater than or equal to the length of TRIM_WITH (", TRIM_WITH.length(), ')');
    wstr_t wstr(wstrsv);
    if (wstr.length() > maxLength)
      return wstr.substr(0, maxLength - TRIM_WITH.length()) + TRIM_WITH.data();
    return wstr;
  }
  wstr_t shorten_begin(wstrsv_t wstrsv, size_t maxLength) {
    /*
      Trim and content of `TRIM_WITH` to the beginning of the string
      if it exceeds `maxLength`.
      Example:
        cslib::shorten_begin(L"cslib.h++", 3);
        // is "...h++"
    */
    if (maxLength < TRIM_WITH.length())
      throw_up("maxLength must be greater than or equal to the length of TRIM_WITH (", TRIM_WITH.length(), ')');
    wstr_t wstr(wstrsv);
    if (wstr.length() > maxLength)
      return TRIM_WITH.data() + wstr.substr(wstr.length() - (maxLength - TRIM_WITH.length()));
    return wstr;
  }



  wstr_t upper(wstrsv_t wstrsv) {
    /*
      Converts it to uppercase.
      Example:
        to_upper("csLib.h++");
        // is "CSLIB.H++"
    */
    wstr_t str(wstrsv);
    return std::transform(str.begin(), str.end(), str.begin(), ::toupper), str;
  }
  void upper_ref(wstr_t& wstr) {
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::toupper);
  }

  wstr_t lower(wstrsv_t wstrsv) {
    /*
      Converts it to lowercase.
      Example:
        to_lower("csLib.h++");
        // is "cslib.h++"
    */
    wstr_t str(wstrsv);
    return std::transform(str.begin(), str.end(), str.begin(), ::tolower), str;
  }
  void lower_ref(wstr_t& wstr) {
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::tolower);
  }



  std::vector<wstr_t> separate(wstrsv_t wstrsv, wchar_t delimiter) {
    /*
      Same as above, but for wide strings.
      Example:
        cslib::separate(L"Hello World", ' ') // {"Hello", "World"}
    */
    wstr_t str(wstrsv);
    std::vector<wstr_t> result;
    wstr_t temp;

    if (str.empty() or delimiter == L'\0')
      return result;

    for (wchar_t c : str) {
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



  size_t roll_dice(size_t min, size_t max) {
    /*
      This function takes a minimum and maximum value and returns a random
      number between them (inclusive).
    */

    if (min > max) std::swap(min, max);

    // Special thanks to copilot. No idea what this does
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(min, max);
    return dis(gen);
  }



  wstr_t input() {
    wstr_t input;
    std::getline(std::wcin, input);
    return input;
  }



  void enable_wchar_io() {
    // Set all io-streaming globally to UTF-8 encoding

    // Get the first available UTF-8 locale
    const std::vector<std::string_view> utf8_locales = {
      "en_US.UTF-8",
      "en_US.utf8",
      "C.UTF-8",
      "POSIX.UTF-8",
      "C.utf8",
      "POSIX.utf8"
    };
    std::locale utf8_locale;
    for (std::string_view locale_name : utf8_locales) {
      try {
        utf8_locale = std::locale(locale_name.data());
      } catch (const std::runtime_error&) {
        // Ignore the exception, try the next locale
      }
    }
    if (utf8_locale.name().empty())
      throw_up("Failed to find a suitable UTF-8 locale. Ensure your system supports UTF-8 locales");

    std::locale::global(utf8_locale);
    std::wcout.imbue(utf8_locale);
    std::wcin.imbue(utf8_locale);
    std::wclog.imbue(utf8_locale);
    std::wcerr.imbue(utf8_locale);

    std::wcout << L"[⛓️‍💥] Console initialized with UTF-8 encoding.\n";
  }



  // Classes
  MACRO Bold = L"\033[1m";
  MACRO Underline = L"\033[4m";
  MACRO Italic = L"\033[3m";
  MACRO Reverse = L"\033[7m";
  MACRO Hidden = L"\033[8m";
  MACRO Black = L"\033[30m";
  MACRO Red = L"\033[31m";
  MACRO Green = L"\033[32m";
  MACRO Yellow = L"\033[33m";
  MACRO Blue = L"\033[34m";
  MACRO Magenta = L"\033[35m";
  MACRO Cyan = L"\033[36m";
  MACRO White = L"\033[37m";
  MACRO Reset = L"\033[0m";
  class Out { public:
    /*
      Print to console with color and optionally into an existing file.
      Usage:
        cslib::Out error("Error: ", cslib::Out::Color::RED);
        error << "Something went wrong";
    */
    wstr_t prefix;
    Out(wstrsv_t wprefsv, wstrsv_t color = L"") {
      prefix = color;
      prefix += wprefsv;
      prefix += Reset;
      prefix += L" ";
    }
    Out(std::string_view prefsv, std::string_view color = "") {
      prefix = to_wstr(color);
      prefix += to_wstr(prefsv);
      prefix += Reset;
      prefix += L" ";
    }
    template <typename T>
    std::wostream& operator<<(const T& msg) {
      /*
        Print to console with the given prefix
        Example:
          cslib::Out out(L"Info: ", GREEN);
          out << "This is an info message" << std::endl;
      */
      std::wcout << prefix << msg << std::flush;
      return std::wcout;
    }
    std::wostream& operator<<(const std::string& msg) = delete;
    std::wostream& operator<<(const std::string_view& msg) = delete;
    std::wostream& operator<<(const char* msg) = delete;
    /*
      Syncing two streambuffers are unreliable and
      requires a lot of workarounds. Enforcing wchar_t
      allocated stream buffers is more unified and
      easier to use
    */
  };



  class TimeStamp { public:
    // A wrapper around std::chrono that I have control over
    std::chrono::system_clock::time_point timePoint;
    TimeStamp() {update();}
    TimeStamp(std::chrono::system_clock::time_point tp) : timePoint(tp) {}
    void update() {
      timePoint = std::chrono::system_clock::now();
    }
    wstr_t asWstr() const {
      /*
        Convert the time point to (lighter form of) ISO 8601
        in format YYYY-MM-DD HH:MM:SS)..
      */
      std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
      return (std::wstringstream() << std::put_time(std::gmtime(&time), L"%Y-%m-%d %H:%M:%S")).str();
    }
  };
  void sleep_until(const TimeStamp& untilPoint) {
    // Sleep until the given time point.
    if (untilPoint.timePoint <= std::chrono::system_clock::now()) {
      throw_up("Cannot sleep until a time point in the past: ", TimeStamp().asWstr(), " (current time: ", untilPoint.asWstr(), ')');
    }
    std::this_thread::sleep_until(untilPoint.timePoint);
  }



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
    size_t elapsed_ms() {
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    }
    size_t elapsed_us() {
      return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    }
    size_t elapsed_ns() {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    }
  };




  #ifdef _WIN32
    MACRO PATH_DELIMITER = L'\\';
  #else
    MACRO PATH_DELIMITER = L'/';
  #endif
  class VirtualPath { public:
    // Wrapper around std::filesystem::path

    std::filesystem::path isAt; 

    VirtualPath() = default;
    VirtualPath(wstrsv_t where) : isAt(std::filesystem::canonical(where.data())) {}
    /*
      Constructor that takes a string and checks if it's a valid path.
      Notes:
        - If where is relative, it will be converted to an absolute path.
        - If where is empty, you will crash.
    */
    VirtualPath(wstrsv_t where, std::filesystem::file_type shouldBe) : VirtualPath(where) {
      if (this->type() != shouldBe) {
        wstr_t errorMsg = L"Path '" + to_wstr(where) + L"' should be of type '";
        switch (shouldBe) {
          case std::filesystem::file_type::regular: errorMsg += L"regular file"; break;
          case std::filesystem::file_type::directory: errorMsg += L"directory"; break;
          case std::filesystem::file_type::symlink: errorMsg += L"symlink"; break;
          case std::filesystem::file_type::block: errorMsg += L"block device"; break;
          case std::filesystem::file_type::character: errorMsg += L"character device"; break;
          case std::filesystem::file_type::fifo: errorMsg += L"FIFO (named pipe)"; break;
          case std::filesystem::file_type::socket: errorMsg += L"socket"; break;
          default: errorMsg += L"unknown type";
        }
        errorMsg += L"', but is actually of type '";
        switch (this->type()) {
          case std::filesystem::file_type::regular: errorMsg += L"regular file"; break;
          case std::filesystem::file_type::directory: errorMsg += L"directory"; break;
          case std::filesystem::file_type::symlink: errorMsg += L"symlink"; break;
          case std::filesystem::file_type::block: errorMsg += L"block device"; break;
          case std::filesystem::file_type::character: errorMsg += L"character device"; break;
          case std::filesystem::file_type::fifo: errorMsg += L"FIFO (named pipe)"; break;
          case std::filesystem::file_type::socket: errorMsg += L"socket"; break;
          default: errorMsg += L"unknown type";
        }
        errorMsg += L"'";
        throw_up(errorMsg);
      }
    }

    std::filesystem::file_type type() const {
      /*
        Returns the type of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          std::filesystem::file_type type = path.type();
          // type = std::filesystem::file_type::regular
      */
      if (isAt.empty())
        throw_up("VirtualPath ", to_ptrstr(this), " isn't initialized");
      return std::filesystem::status(isAt).type();
    }
    VirtualPath parent() const {
      /*
        Returns the parent path of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          VirtualPath parent = path.parent_path();
          // parent = "/gitstuff/cslib"
      */
      if (isAt.empty())
        throw_up("VirtualPath ", to_ptrstr(this), " isn't initialized");
      if (isAt.parent_path().empty())
        throw_up("VirtualPath '", to_ptrstr(this), "' has somehow no parent");
      return VirtualPath(isAt.parent_path().wstring(), std::filesystem::file_type::directory);
    }
    size_t depth() const {
      /*
        Returns the depth of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          size_t depth = path.depth();
          // depth = 2 (because there are 2 directories before the file)
      */
      if (isAt.empty())
        throw_up("VirtualPath ", to_ptrstr(this), " isn't initialized");
      return separate(isAt.wstring(), PATH_DELIMITER).size() - 1; // -1 for the last element
    }
    TimeStamp last_modified() const {
      /*
        Last modified date of this instance.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          TimeStamp lastModified = path.last_modified();
          // lastModified = 2023-10-01 12:34:56
        Note:
          Once more, special thanks to copilot. I scurried
          through the std::filesystem documentation and
          couldn't find a usable way to get the last modified
          date of a file.
      */
      std::filesystem::file_time_type ftime = std::filesystem::last_write_time(isAt);
      std::chrono::system_clock::time_point timePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
      std::time_t cftime = std::chrono::system_clock::to_time_t(timePoint);
      struct tm* timeinfo = std::localtime(&cftime);
      TimeStamp ts(std::chrono::system_clock::from_time_t(std::mktime(timeinfo)));
      return ts;
    }
    void move_to(VirtualPath moveTo) {
      // Move this instance to a new location and apply changes.
      if (this->isAt.empty())
        throw_up("VirtualPath ", to_ptrstr(this), " isn't initialized");
      if (moveTo.isAt.empty())
        throw_up("Target path is empty (this: '", this->isAt.wstring(), "', target: '", moveTo.isAt.wstring(), "')");
      if (moveTo.type() != std::filesystem::file_type::directory)
        throw_up("Target path '", moveTo.isAt.wstring(), "' is not a directory (this: '", this->isAt.wstring(), "')");
      if (moveTo.isAt == this->isAt)
        throw_up("Cannot move to the same path: ", this->isAt.wstring());
      wstr_t willBecome = moveTo.isAt.wstring() + to_wstr(PATH_DELIMITER) + this->isAt.filename().wstring();
      if (std::filesystem::exists(willBecome))
        throw_up("Target path '", willBecome, "' already exists (this: '", this->isAt.wstring(), "')");
      std::filesystem::rename(this->isAt, willBecome);
      this->isAt = VirtualPath(willBecome).isAt; // Apply changes
    }
    VirtualPath copy_into(VirtualPath targetDict) const {
      /*
        Copies this instance to a new location and returns
        a new VirtualPath instance pointing to the copied file.
      */
      if (this->isAt.empty())
        throw_up("VirtualPath ", to_ptrstr(this), " isn't initialized");
      if (targetDict.isAt.empty())
        throw_up("Target path is empty (this: '", this->isAt.wstring(), "', target: '", targetDict.isAt.wstring(), "')");
      if (targetDict.type() != std::filesystem::file_type::directory)
        throw_up("Target path '", targetDict.isAt.wstring(), "' is not a directory (this: '", this->isAt.wstring(), "')");
      if (targetDict.isAt == this->isAt)
        throw_up("Cannot copy to the same path: ", this->isAt.wstring());
      wstr_t willBecome = targetDict.isAt.wstring() + to_wstr(PATH_DELIMITER) + this->isAt.filename().wstring();
      if (std::filesystem::exists(willBecome))
        throw_up("Element '", willBecome, "' already exists in target directory (this: '", this->isAt.wstring(), "')");
      std::filesystem::copy(this->isAt, willBecome);
      return VirtualPath(willBecome);
    }
  };



  class File { public:
    /*
      Child class of VirtualPath that represents a file.
      Example:
        File file("/gitstuff/cslib/cslib.h++");
        std::string content = file.content();
        // content = "Hello World"
    */

    VirtualPath is; // Composition over inheritance

    File() = default;
    File(wstrsv_t where) : is(where, std::filesystem::file_type::regular) {}

    wstr_t content(std::ios_base::openmode openMode = std::ios::in) const {
      /*
        Read the content of the file and return it as a string.
        Note:
          - No error-handling for files larger than available memory
      */
      if (is.isAt.empty())
        throw_up("Uninitialized path for file at ", to_ptrstr(this));
      std::wifstream file(is.isAt, openMode);
      if (!file.is_open())
        throw_up("Failed to open file '", is.isAt.wstring(), '\'');
      if (!file.good())
        throw_up("Failed to read file '", is.isAt.wstring(), '\'');
      return wstr_t((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
    }
    std::string binary_content(std::ios_base::openmode openMode = std::ios::binary) const {
      /*
        Read the content of the file and return it as a binary string.
        Note:
          - No error-handling for files larger than available memory
      */
      if (is.isAt.empty())
        throw_up("Uninitialized path for file at ", to_ptrstr(this));
      std::ifstream file(is.isAt, openMode);
      if (!file.is_open())
        throw_up("Failed to open file '", is.isAt.wstring(), '\'');
      if (!file.good())
        throw_up("Failed to read file '", is.isAt.wstring(), '\'');
      return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    wstr_t wstr() const {
      if (is.isAt.empty())
        throw_up("Uninitialized path for file at ", to_ptrstr(this));
      return is.isAt.wstring();
    }
    wstr_t extension() const {
      if (is.isAt.empty())
        throw_up("Uninitialized path for file at ", to_ptrstr(this));
      return is.isAt.extension().wstring();
    }
    size_t bytes() const {
      if (is.isAt.empty())
        throw_up("Uninitialized path for file at ", to_ptrstr(this));
      return std::filesystem::file_size(is.isAt);
    }
  };



  class Folder { public:
    /*
      Child class of VirtualPath that represents a folder and
      everything in it.
    */
    std::vector<VirtualPath> content;
    VirtualPath is;

    Folder() = default;
    Folder(wstrsv_t where) : is(where, std::filesystem::file_type::directory) {update();}
    wstr_t wstr() const {
      if (is.isAt.empty())
        throw_up("Uninitialized path for folder at ", to_ptrstr(this));
      return is.isAt.wstring();
    }
    void update() {
      if (is.isAt.empty())
        throw_up("Uninitialized path for folder at ", to_ptrstr(this));
      content.clear();
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(is.isAt))
        content.push_back(VirtualPath(entry.path().wstring()));
      content.shrink_to_fit();
    }
  };




  // Namespaces
  namespace TinySTL {
    // The stl is good but sometimes you need something smaller

    template <typename T>
    class Vector { public:
      /*
        Same as std::vector but smaller and less features.
        Note:
          The size and capacity are 4 bytes each, when padding
          is done, memory-usage isn't doubled.
      */

      T* data;
      uint32_t size;
      uint32_t capacity;

      Vector() : data(nullptr), size(0), capacity(0) {}
      Vector(uint32_t initialCapacity) : size(0), capacity(initialCapacity) {
        if (initialCapacity == 0)
          throw_up("Vector capacity must be greater than 0");
        data = new T[initialCapacity];
      }
      Vector(std::initializer_list<T> initList) : size(initList.size()), capacity(initList.size()) {
        // Copies whatever is in the initializer list
        if (initList.size() == 0)
          throw_up("Vector initializer list cannot be empty");
        data = new T[initList.size()];
        uint32_t index = 0;
        for (const T& item : initList)
          data[index++] = item;
      }
      Vector(const std::vector<T>& vec) : size(vec.size()), capacity(vec.capacity()) {
        // Copies whatever is in the std::vector
        if (vec.empty())
          throw_up("Vector cannot be initialized from an empty std::vector");
        data = new T[vec.size()];
        uint32_t index = 0;
        for (const T& item : vec)
          data[index++] = item;
      }
      Vector(const Vector&) = delete; // No copy constructor
      Vector& operator=(const Vector&) = delete; // No copy assignment operator
      Vector(Vector&& other) noexcept : data(other.data), size(other.size), capacity(other.capacity) {
        other.data = nullptr; // Prevent deletion of moved-from data
        other.size = 0;
        other.capacity = 0;
      }
      Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
          delete[] data;
          data = other.data;
          size = other.size;
          capacity = other.capacity;
          other.data = nullptr; // Prevent deletion of moved-from data
          other.size = 0;
          other.capacity = 0;
        }
        return *this;
      }
      ~Vector() { delete[] data; }

      T* begin() { return data; }
      T* end() { return data + size; }
      void increment_capacity() {
        T* new_data = new T[++capacity];
        size = -1;
        for (T& ownData : *this)
          new_data[++size] = ownData;
        delete[] data;
        data = new_data;
      }
      void push_back(T value) {
        if (size == capacity)
          increment_capacity();
        data[++size] = value;
      }
      T& operator[](uint32_t index) {
        if (index >= size)
          throw_up("Index out of bounds for Vector at ", to_ptrstr(this), ": ", index, " >= ", size);
        return data[index];
      }
      void pop_back() {
        if (size == 0)
          throw_up("Cannot pop from an empty vector at ", to_ptrstr(this));
        --size;
      }
    };



    template <uint8_t N>
    class String { public:
      /*
        Static fixed size string. It can hold up to `N` characters and is
        only null-terminated when cslib::String's capacity isn't maxed.
        If it is, the size-limit acts as a terminator.
        Example:
          String<2> str("Hi");
          // str = {'H', 'i'}
          String<3> str2("Hi");
          // str2 = {'H', 'i', '\0'}
          String<4> str3("Hi");
          // str3 = {'H', 'i', '\0', 'm' ('m' could have been used
          for something else earlier but ignored after null-termination)}
      */
      static_assert(N > 0, "String size must be greater than 0");

      char data[N] = {'\0'};

      String() = default;
      String(std::string_view str) {
        for (char c : str)
          append(c);
      }
      template <uint8_t ON> // Other N
      String(const String<ON>&) = delete;
      template <uint8_t ON>
      String& operator=(const String<ON>&) = delete;
      String(String&& other) noexcept {
        for (uint8_t i = 0; i < N; ++i)
          data[i] = other.data[i];
        other.clear();
      }
      String& operator=(String&& other) noexcept {
        if (this != &other) {
          for (uint8_t i = 0; i < N; ++i)
            data[i] = other.data[i];
          other.clear();
        }
        return *this;
      }

      uint8_t length() {
        uint8_t size = 0;
        while (data[size] != '\0' and size < N)
          ++size;
        return size;
      }
      void append(char c) {
        uint8_t size = length();
        if (size >= N)
          throw_up("String (", to_ptrstr(this), ") capacity exceeded: ", N);
        data[size] = c;
        if (size + 1 < N)
          data[size + 1] = '\0';
      }
      void clear() {
        data = {'\0'};
      }
      char& at(uint8_t index) {
        if (index >= N)
          throw_up("Index out of bounds for String at ", to_ptrstr(this), ": ", index, " >= ", N);
        return data[index];
      }
      char* begin() {return data;}
      char* end() {return data + length();}
      std::string std_str() {
        std::string str;
        for (char c : data)
          str += c;
        return str;
      }
      bool operator==(String other) {
        return this->std_str() == other.std_str();
      }
      bool operator==(std::string_view other) {
        return this->std_str() == other.data();
      }
    };
    template <uint8_t N>
    class Wstring { public:
      /*
        Same as String but for wide characters.
        Example:
          Wstring<2> str(L"Hi");
          // str = {L'H', L'i'}
          Wstring<3> str2(L"Hi");
          // str2 = {L'H', L'i', L'\0'}
          Wstring<4> str3(L"Hi");
          // str3 = {L'H', L'i', L'\0', L'm' (L'm' could have been used
          for something else earlier but ignored after null-termination)}
      */
      static_assert(N > 0, "Wstring size must be greater than 0");

      wchar_t data[N] = {L'\0'};

      Wstring() = default;
      Wstring(std::wstring str) {
        for (wchar_t c : str)
          append(c);
      }
      template <uint8_t ON> // Other N
      Wstring(const Wstring<ON>&) = delete;
      template <uint8_t ON>
      Wstring& operator=(const Wstring<ON>&) = delete;
      Wstring(Wstring&& other) noexcept {
        // Move constructor
        for (uint8_t i = 0; i < N; ++i)
          data[i] = other.data[i];
        other.clear();
      }
      Wstring& operator=(Wstring&& other) noexcept {
        if (this != &other) {
          for (uint8_t i = 0; i < N; ++i)
            data[i] = other.data[i];
          other.clear();
        }
        return *this;
      }

      uint8_t length() {
        uint8_t size = 0;
        while (data[size] != L'\0' and size < N)
          ++size;
        return size;
      }
      void append(wchar_t c) {
        uint8_t size = length();
        if (size >= N)
          throw_up("Wstring (", to_ptrstr(this), ") capacity exceeded: ", N);
        data[size] = c;
        if (size + 1 < N)
          data[size + 1] = L'\0';
      }
      void clear() {
        data = {L'\0'};
      }
      wchar_t& at(uint8_t index) {
        if (index >= N)
          throw_up("Index out of bounds for Wstring at ", to_ptrstr(this), ": ", index, " >= ", N);
        return data[index];
      }
      wchar_t* begin() {return data;}
      wchar_t* end() {return data + length();}
      wstr_t std_str() {
        wstr_t str;
        for (wchar_t c : data)
          str += c;
        return str;
      }
    };


    template <typename K, typename V>
    class Map { public:
      /*
        A TinySTL::Vector that holds key-value pairs.
      */
      struct Node {
        K key;
        V value;
      };

      Vector<Node> data;

      Map() = default;
      Map(size_t initialCapacity) : data(initialCapacity) {
        if (initialCapacity == 0)
          throw_up("Map capacity must be greater than 0");
      }
      Map(const Map&) = delete;
      Map& operator=(const Map&) = delete;
      Map(Map&& other) noexcept : data(std::move(other.data)) {
        other.data = Vector<Node>();
      }
      Map& operator=(Map&& other) noexcept {
        if (this != &other) {
          data = std::move(other.data);
          other.data = Vector<Node>();
        }
        return *this;
      }

      void insert(K key, V value) {
        if (contains(key))
          throw_up("Key already exists for Map at ", to_ptrstr(this));
        data.push_back({key, value});
      }
      V& at(K key) {
        for (Node& node : data)
          if (node.key == key)
            return node.value;
        throw_up("Key not found for Map at ", to_ptrstr(this));
      }
      V& operator[](K key) {
        return at(key);
      }
      bool contains(K key) const {
        for (Node node : data)
          if (node.key == key)
            return true;
        return false;
      }
      std::pair<K, V>* begin() {
        return reinterpret_cast<std::pair<K, V>*>(data.begin());
      }
      std::pair<K, V>* end() {
        return reinterpret_cast<std::pair<K, V>*>(data.end());
      }
    };
  };
} // namespace cslib