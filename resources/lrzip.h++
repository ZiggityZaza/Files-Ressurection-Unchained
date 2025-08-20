#include "./cslib.h++"
#pragma once

namespace LRZTAR {
  using namespace cslib;
  /*
    Compress and archive folders from the exchange folder so
    that they can be restored later
    Note:
      We use lrztar (from lrzip) for compression
    Important note:
      At compression, it matters how the path is specified. lrztar will
      also contain all the parent paths of the source directory. So
      if you specify the path as "/home/user/docs", the compressed file
      will contain the parent directories "home" and "user" as empty
      directories as well. Example:
      /home (literally only user directory)
        └ user (also just the docs directory)
          └ docs (contains the files)
            ├ what ever you put in here
            └ ...
      I haven't found a way to change this. If you want to compress a
      directory, make it inside the same directory as the executable.
      This doesn't matter for decompression. Decompression also acts
      differently. To be sure, decompress within the same directory
      as the binary and the compressed file
  */

  class fru_error : public std::runtime_error { public:
    /*
      Custom error class for LRZTAR
      Example:
        throw LRZTAR::fru_error("Something went wrong");
    */
    fru_error(size_t _lineInCode, const auto&... _msgs) : std::runtime_error([_lineInCode, &_msgs...] {
      /*
        Create a custom error message with the given messages.
        Example:
          throw cslib::any_error(__LINE__, "Aye", L"yo", '!', 123, true);
      */
      std::ostringstream oss;
      oss << "LRZTAR::fru_error called in workspace " << std::filesystem::current_path();
      oss << " on line " << _lineInCode << " because: ";
      ((oss << to_str(std::forward<decltype(_msgs)>(_msgs))), ...);
      oss << std::flush;
      return oss.str();
    }()) {}
  };
  #define throw_up(...) throw LRZTAR::fru_error(__LINE__, __VA_ARGS__)

  enum class BackupChanges {
    MODIFIED,
    ADDED,
    DELETED
  };
  FIXED str_t name_BackupChanges(BackupChanges _backupChanges) {
    switch (_backupChanges) {
      case BackupChanges::MODIFIED: return "MODIFIED";
      case BackupChanges::ADDED:    return "ADDED";
      case BackupChanges::DELETED:  return "DELETED";
      default:                      return "UNKNOWN";
    }
  }
  FIXED BackupChanges name_BackupChanges(str_t _identifier) {
    if (_identifier == "MODIFIED") return BackupChanges::MODIFIED;
    if (_identifier == "ADDED")    return BackupChanges::ADDED;
    if (_identifier == "DELETED")  return BackupChanges::DELETED;
    throw_up("Unknown BackupChanges identifier: ", _identifier);
  }

  Folder decompress(File) = delete;
  Folder decompress(Folder) = delete;
  Folder decompress() = delete;
  // No support for decompression yet due to excessive complexity

  MACRO LRZCAT_CMD_HEAD = "lrzcat \"";
  MACRO LRZCAT_CMD_TAIL = "\" | tar -tv";
  MACRO COMPRESS_CMD_HEAD = "lrztar -z \"";
  MACRO COMPRESS_CMD_TAIL = "\"";
  MACRO EXPECTED_EXTENSION = ".tar.lrz";
  // Out out = Out(std::cout, "[FRU]:", Magenta);
  Out make_out(str_t funcName) {
    const char* color;
    switch (roll_dice(0, 5)) {
      case 0: color = Red; break;
      case 1: color = Green; break;
      case 2: color = Yellow; break;
      case 3: color = Blue; break;
      case 4: color = Magenta; break;
      case 5: color = Cyan; break;
      default: throw_up("Unkown color");
    }
    return Out(std::cout, "[FRU " + funcName + "]:", color);
  }
  const Folder EXCHANGE_FOLDER("/root/fru_shared"); // Grab targets from (immutable)
  // Folder BACKUP_FOLDER("/root/fru_archive"); // Put compressed files in here
  TempFolder BACKUP_FOLDER; // Debugging alternative to above clean itself up
  Folder WORKING_DIR(std::filesystem::current_path());




  std::map<str_t, Folder> existing_backup_groups() {
    /*
      Note:
        The first element the full-back up. Every
        following are incremental changes.
    */
    std::map<str_t, Folder> backups;
    for (const Road& _entry : BACKUP_FOLDER.list())
      if (!backups.insert({_entry.name(), Folder(_entry)}).second)
        throw_up("Duplicate backup group found: ", _entry.name());
    return backups;
  }



  std::vector<std::pair<Road, BackupChanges>> list_changes_from_full_backup(Folder _backupGroup, Folder _compareTo) {
    /*
      Create a list of changes from the full backup
      Example:
        file.txt [MODIFIED]
        folder/subfile.txt [ADDED]
        folder/subfolder [DELETED]
    */
    std::vector<std::pair<Road, BackupChanges>> changes;
    for (std::variant<File, Folder, Road> _entry : _backupGroup.typed_list()) {
      BackupChanges change = determine_change_type(_entry, _compareTo);
      changes.emplace_back(_entry, change);
    }
    return changes;
  }



  std::vector<Folder> get_all_folders_in_exchange() {
    Out out = make_out(__func__);
    std::vector<Folder> folders;
    for (Road entry : EXCHANGE_FOLDER.list()) {
      folders.emplace_back(entry); // Implicit error handling
      out << "Found folder " << folders.back() << " in exchange folder " << EXCHANGE_FOLDER << '\n';
    }
    out << "Total of " << folders.size() << " folders found in exchange folder " << EXCHANGE_FOLDER << '\n';
    return folders;
  }


  Folder carry_copy_in_here(Folder _toBeCompressed) {
    /*
      Copy the folder that is supposed to be handled by
      this program to this directory
      Note:
        Target MUST be in the Exchange Folder
      Example:
        carry_copy_in_here(Folder(L"/root/shared/docs"));
        // Copies the folder "docs" to "/root/shared"
    */
    Out out = make_out(__func__);
    if (!EXCHANGE_FOLDER.has(_toBeCompressed))
      throw_up("Target folder ", _toBeCompressed, " isn't in the exchange folder ", EXCHANGE_FOLDER); 
    if (WORKING_DIR.has(_toBeCompressed))
      throw_up("Target folder ", _toBeCompressed, " is already in the working directory ", WORKING_DIR);

    out << "Copying folder " << _toBeCompressed << " to working directory " << WORKING_DIR << '\n';
    Folder copyOfToBeCompressed = _toBeCompressed.copy_self_into(WORKING_DIR);
    out << "Copied folder " << _toBeCompressed << " to working directory " << WORKING_DIR << '\n';
    if (!WORKING_DIR.has(copyOfToBeCompressed))
      throw_up("Failed to copy folder ", _toBeCompressed, " to working directory ", WORKING_DIR);
    return copyOfToBeCompressed;
  }


  File compress(Folder& _toBeCompressed) {
    /*
      Compress the folder and return the compressed
      .tar.lrz file
      Example:
        File compressedFile = compress(Folder(L"./docs"));
        // Compressed file is "./docs.tar.lrz"
    */
    Out out = make_out(__func__);

    if (!WORKING_DIR.has(_toBeCompressed))
      throw_up("Folder ", _toBeCompressed, " isn't in the working directory ", WORKING_DIR);

    str_t willBecome = _toBeCompressed.str() + EXPECTED_EXTENSION;
    if (std::filesystem::exists(willBecome))
      throw_up("Compressed file ", willBecome, " already exists in the working directory ", WORKING_DIR);

    out << "Compressing folder " << _toBeCompressed << " to " << willBecome << '\n';
    sh_call(COMPRESS_CMD_HEAD + _toBeCompressed.str() + COMPRESS_CMD_TAIL);
    out << "Compressed folder " << _toBeCompressed << " to " << willBecome << '\n';
    File willBecomeFile(willBecome);
    if (!WORKING_DIR.has(willBecomeFile))
      throw_up("Failed to compress folder ", _toBeCompressed, " to ", willBecome, " in working directory ", WORKING_DIR);
    std::filesystem::remove_all(_toBeCompressed); // Remove the copy
    return willBecomeFile;
  }


  void archive(File& _compressedFile) {
    /*
      Move the compressed file to the backup folder
      Example:
        File compressedFile(L"./docs.tar.lrz");
        archive(compressedFile);
        // Moves the file to "/root/archive/docs.tar.lrz"
    */
    Out out = make_out(__func__);

    if (!WORKING_DIR.has(_compressedFile))
      throw_up("Compressed file ", _compressedFile, " isn't in the working directory ", WORKING_DIR);
    if (BACKUP_FOLDER.has(_compressedFile))
      throw_up("Compressed file ", _compressedFile, " is already in the backup folder ", BACKUP_FOLDER);

    _compressedFile.rename_self_to(_compressedFile.name() + "@" + TimeStamp().as_str());
    _compressedFile.move_self_into(BACKUP_FOLDER);
    if (!BACKUP_FOLDER.has(_compressedFile))
      throw_up("Failed to move compressed file ", _compressedFile, " to backup folder ", BACKUP_FOLDER);
    out << "Archived compressed file " << _compressedFile << " to " << BACKUP_FOLDER << '\n';
  }


  void pull_and_archive(const Folder& _toBeCompressed) {
    /*
      Pull the folder from the exchange folder,
      compress it and archive it
      Example:
        pull_and_archive(Folder(L"./docs"));
        // Pulls the folder "docs" from "/root/shared", compresses it and archives it
      Note:
        Takes care of moving, compressing and archiving
    */
    Out out = make_out(__func__);

    out << "Pulling folder " << _toBeCompressed << " from exchange folder " << EXCHANGE_FOLDER << '\n';
    Folder copyOfToBeCompressed = carry_copy_in_here(_toBeCompressed);
    File compressedFile = compress(copyOfToBeCompressed);
    archive(compressedFile);
    out << "Done! Compressed file is " << compressedFile << '\n';
  }


  void go_back_to_sleep() {
    /*
      After compressing and archiving the
      folders, hibernate the program until
      the next run on the 1st of the next
      month at 3 AM
      Note:
        3 am is the most probable time
        that the system is idle, so the
        program can run without the user
        can notice performance issues.
      Example:
        go_back_to_sleep();
        // Hibernates the program until the next run
    */
    Out out = make_out(__func__);

    out << "Going back to sleep...\n";
    uint currentMonth = TimeStamp().month();
    uint targetYear = TimeStamp().year();
    uint nextMonth = currentMonth + 1;
    if (nextMonth > 12) {
      nextMonth = 1;
      ++targetYear;
    }
    TimeStamp nextRun(
      0,
      0,
      3,
      1,
      nextMonth,
      targetYear
    );
    out << "Next run will be on " << nextRun.as_str() << "\n";
    // Hibernate the program until the next run
    std::this_thread::sleep_until(nextRun.timePoint);
  }
};