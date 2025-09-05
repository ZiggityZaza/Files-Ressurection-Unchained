#include "../CeggsyLib/cslib.h++" // Adjust path as needed (get it on Github ZiggityZaza/CeggsyLib)



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

enum BackupChanges {
  MODIFIED,
  ADDED,
  DELETED
};
FIXED maybe<strv_t> to_str(BackupChanges _identifier) noexcept {
  switch (_identifier) {
    case BackupChanges::MODIFIED: return "MODIFIED";
    case BackupChanges::ADDED:    return "ADDED";
    case BackupChanges::DELETED:  return "DELETED";
    default: return unexpect("Unknown BackupChanges value: ", _identifier);
  }
}
FIXED maybe<BackupChanges> str_to_backupChanges(str_t _identifier) {
  if (_identifier == "MODIFIED") return BackupChanges::MODIFIED;
  if (_identifier == "ADDED")    return BackupChanges::ADDED;
  if (_identifier == "DELETED")  return BackupChanges::DELETED;
  return unexpect("Unknown BackupChanges identifier: ", _identifier);
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
Out make_out(str_t funcName, strv_t _color = "") noexcept {
  return Out(std::cout, "[FRU " + funcName + "]:", _color);
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
  for (Road entry : BACKUP_FOLDER.untyped_list()) {
    Road entryBase = entry;
    if (!backups.insert({entryBase.name(), Folder(entryBase)}).second)
      cslib_throw_up("Impossible duplicate backup group found: ", entryBase.name());
  }
  return backups;
}




using notepad_t = std::vector<std::pair<road_t, BackupChanges>>;
void note_deleted_raw(notepad_t& _notepad, Folder _original, Folder _current) {
  /*
    Note which files/folders are missing
    Example:
      std::vector<std::pair<Folder::list_t, BackupChanges>> changes;
      note_deleted_raw(changes, "./original", "./current");
      changes == {
        {"folder/subfolder", DELETED},
        ...
      }
  */
  for (road_t entry : _original.list()) {
    if (holds<File>(entry))
      if (!_current.find(get<File>(entry).name()))
        _notepad.push_back({entry, DELETED});

    if (holds<Folder>(entry)) {
      maybe<Folder> _subFolder(get<Folder>(get(_current.find(get<Folder>(entry).name()))));
      if (_subFolder.has_value())
        note_deleted_raw(_notepad, Folder(get(_subFolder)), get<Folder>(entry));
      else
        _notepad.push_back({entry, DELETED});
    }

    if (holds<BizarreRoad>(entry))
      cslib_throw_up("Bizarre file types such as '", get<BizarreRoad>(entry).str(), "' aren't supported yet.");
  }
}



std::vector<Folder> get_all_folders_in_exchange() {
  Out out = make_out(__func__);
  std::vector<Folder> folders;
  for (road_t entry : EXCHANGE_FOLDER.list()) {
    folders.emplace_back(ignore_road_t(entry)); // Implicit error handling
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
  if (!EXCHANGE_FOLDER.find(_toBeCompressed))
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




int main() {
  while (false) {
    // Step 1: Get all folders in the exchange folder
    std::vector<Folder> folders = get_all_folders_in_exchange();

    for (const Folder& folder : folders)
      pull_and_archive(folder);

    go_back_to_sleep(); // Hibernate
  }
}