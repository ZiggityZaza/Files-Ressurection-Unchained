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

  // MACRO DECOMPRESS_CMD_HEAD = "lrztar -d ";
  // MACRO DECOMPRESS_CMD_TAIL = "\"";
  Folder decompress(const File&) = delete;
  Folder decompress(const Folder&) = delete;
  Folder decompress() = delete;
  // No support for decompression due to excessive complexity

  MACRO LRZCAT_CMD_HEAD = "lrzcat \"";
  MACRO LRZCAT_CMD_TAIL = "\" | tar -tv";
  MACRO COMPRESS_CMD_HEAD = "lrztar -z \"";
  MACRO COMPRESS_CMD_TAIL = "\""; // Keep it a string literal, so it can be concatenated with the path
  MACRO EXPECTED_EXTENSION = ".tar.lrz";
  const Out out = Out(std::cout, "[FRU]:", Magenta);
  const Folder EXCHANGE_FOLDER("/root/fru_shared"); // Grab targets from (immutable)
  Folder BACKUP_FOLDER("/root/fru_archive"); // Put compressed files in here
  Folder WORKING_DIR(std::filesystem::current_path().wstring());



  std::vector<Folder> get_all_folders_in_exchange() {
    std::vector<Folder> folders;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(EXCHANGE_FOLDER.wstr())) {
      if (!entry.is_directory())
        throw_up("Entry ", entry.path().wstring(), " in exchange folder is not a directory");
      folders.push_back(Folder(entry.path()));
      out << "Found folder " << folders.back() << " in exchange folder " << EXCHANGE_FOLDER << '\n';
    }
    out << "Total of " << folders.size() << " folders found in exchange folder " << EXCHANGE_FOLDER << '\n';
    return folders;
  }


  Folder carry_copy_in_here(const Folder& toBeCompressed) {
    /*
      Copy the folder that is supposed to be handled by
      this program to this directory
      Note:
        Target MUST be in the Exchange Folder
      Example:
        carry_copy_in_here(Folder(L"/root/shared/docs"));
        // Copies the folder "docs" to "/root/shared"
    */
    if (!EXCHANGE_FOLDER.has(toBeCompressed))
      throw_up("Target folder ", toBeCompressed, " isn't in the exchange folder ", EXCHANGE_FOLDER); 
    if (WORKING_DIR.has(toBeCompressed))
      throw_up("Target folder ", toBeCompressed, " is already in the working directory ", WORKING_DIR);

    out << "Copying folder " << toBeCompressed << " to working directory " << WORKING_DIR << '\n';
    Folder copyOfToBeCompressed = toBeCompressed.copy_self_into(WORKING_DIR);
    out << "Copied folder " << toBeCompressed << " to working directory " << WORKING_DIR << '\n';
    if (!WORKING_DIR.has(copyOfToBeCompressed))
      throw_up("Failed to copy folder ", toBeCompressed, " to working directory ", WORKING_DIR);
    return copyOfToBeCompressed;
  }


  File compress(const Folder& toBeCompressed) {
    /*
      Compress the folder and return the compressed
      .tar.lrz file
      Example:
        File compressedFile = compress(Folder(L"./docs"));
        // Compressed file is "./docs.tar.lrz"
    */
    if (!WORKING_DIR.has(toBeCompressed))
      throw_up("Folder ", toBeCompressed, " isn't in the working directory ", WORKING_DIR);

    str_t willBecome = toBeCompressed.str() + EXPECTED_EXTENSION;
    if (std::filesystem::exists(willBecome))
      throw_up("Compressed file ", willBecome, " already exists in the working directory ", WORKING_DIR);

    out << "Compressing folder " << toBeCompressed << " to " << willBecome << '\n';
    sh_call(COMPRESS_CMD_HEAD + to_str(toBeCompressed.wstr()) + COMPRESS_CMD_TAIL);
    out << "Compressed folder " << toBeCompressed << " to " << willBecome << '\n';
    File willBecomeFile(willBecome);
    if (!WORKING_DIR.has(willBecomeFile))
      throw_up("Failed to compress folder ", toBeCompressed, " to ", willBecome, " in working directory ", WORKING_DIR);
    return willBecomeFile;
  }


  void archive(File& compressedFile) {
    /*
      Move the compressed file to the backup folder
      Example:
        File compressedFile(L"./docs.tar.lrz");
        archive(compressedFile);
        // Moves the file to "/root/archive/docs.tar.lrz"
    */
    if (!WORKING_DIR.has(compressedFile))
      throw_up("Compressed file ", compressedFile, " isn't in the working directory ", WORKING_DIR);
    if (BACKUP_FOLDER.has(compressedFile))
      throw_up("Compressed file ", compressedFile, " is already in the backup folder ", BACKUP_FOLDER);

    compressedFile.move_self_into(BACKUP_FOLDER);
    if (!BACKUP_FOLDER.has(compressedFile))
      throw_up("Failed to move compressed file ", compressedFile, " to backup folder ", BACKUP_FOLDER);
    out << "Archived compressed file " << compressedFile << " to " << BACKUP_FOLDER << '\n';
  }


  void pull_and_archive(const Folder& toBeCompressed) {
    /*
      Pull the folder from the exchange folder,
      compress it and archive it
      Example:
        pull_and_archive(Folder(L"./docs"));
        // Pulls the folder "docs" from "/root/shared", compresses it and archives it
      Note:
        Takes care of moving, compressing and archiving
    */
    out << "Pulling folder " << toBeCompressed << " from exchange folder " << EXCHANGE_FOLDER << '\n';
    Folder copyOfToBeCompressed = carry_copy_in_here(toBeCompressed);
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