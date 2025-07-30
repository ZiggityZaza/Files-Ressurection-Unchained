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
  MACRO EXPECTED_EXTENSION = L".tar.lrz";
  const Out out = Out(std::wcout, L"[FRU]:", Magenta);
  const Folder EXCHANGE_FOLDER(L"/root/fru_shared"); // Grab targets from (immutable)
  Folder BACKUP_FOLDER(L"/root/fru_archive"); // Put compressed files in here
  Folder WORKING_DIR(std::filesystem::current_path().wstring());



  std::vector<Folder> get_all_folders_in_exchange() {
    /*
      Step 1: Get all folders in the exchange folder
    */
    std::vector<Folder> folders;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(EXCHANGE_FOLDER.wstr())) {
      if (!entry.is_directory())
        throw_up("Entry ", entry.path().wstring(), " in exchange folder is not a directory");
      folders.push_back(Folder(entry.path().wstring()));
    }
    return folders;
  }


  Folder carry_here(const Folder& toBeCompressed) {
    /*
      Step 2: Copy the folder that is supposed to be
      handled by this program to this directory
      Note:
        Target MUST be in the Exchange Folder
      Example:
        carry_here(Folder(L"/root/shared/docs"));
        // Copies the folder "docs" to "/root/shared"
    */
    if (!EXCHANGE_FOLDER.has(toBeCompressed))
      throw_up("Target folder '", toBeCompressed.wstr(), "' isn't in the exchange folder '", EXCHANGE_FOLDER.wstr(), "'"); 
    if (WORKING_DIR.has(toBeCompressed))
      throw_up("Target folder '", toBeCompressed.wstr(), "' is already in the working directory '", WORKING_DIR.wstr(), "'");

    out << "Copying folder '" << toBeCompressed.wstr() << "' to working directory '" << WORKING_DIR.wstr() << "'";
    Folder copyOfToBeCompressed = toBeCompressed.copy_self_into(WORKING_DIR);
    out << "Copied folder '" << toBeCompressed.wstr() << "' to working directory '" << WORKING_DIR.wstr() << "'\n";
    if (!WORKING_DIR.has(copyOfToBeCompressed))
      throw_up("Failed to copy folder '", toBeCompressed.wstr(), "' to working directory '", WORKING_DIR.wstr(), "'");
    return copyOfToBeCompressed;
  }


  File compress(const Folder& toBeCompressed) {
    /*
      Step 3: Compress the folder and return
      the compressed .tar.lrz file
      Example:
        File compressedFile = compress(Folder(L"./docs"));
        // Compressed file is "./docs.tar.lrz"
    */
    if (!WORKING_DIR.has(toBeCompressed))
      throw_up("Folder '", toBeCompressed.wstr(), "' isn't in the working directory '", WORKING_DIR.wstr(), "'");
    
    wstr_t willBecome = toBeCompressed.wstr() + EXPECTED_EXTENSION;
    if (std::filesystem::exists(willBecome))
      throw_up("Compressed file '", willBecome, "' already exists in the working directory '", WORKING_DIR.wstr(), "'");

    out << "Compressing folder '" << toBeCompressed.wstr() << "' to '" << willBecome << "'";
    sh_call(COMPRESS_CMD_HEAD + to_str(toBeCompressed.wstr()) + COMPRESS_CMD_TAIL);
    out << "Compressed folder '" << toBeCompressed.wstr() << "' to '" << willBecome << "'\n";
    File willBecomeFile(willBecome);
    if (!WORKING_DIR.has(willBecomeFile))
      throw_up("Failed to compress folder '", toBeCompressed.wstr(), "' to '", willBecome, "'");
    return willBecomeFile;
  }


  void archive(File& compressedFile) {
    /*
      Step 4: Move the compressed file
      to the backup folder
      Example:
        File compressedFile(L"./docs.tar.lrz");
        archive(compressedFile);
        // Moves the file to "/root/archive/docs.tar.lrz"
    */
    if (!WORKING_DIR.has(compressedFile))
      throw_up("Compressed file '", compressedFile.wstr(), "' isn't in the working directory '", WORKING_DIR.wstr(), "'");
    if (BACKUP_FOLDER.has(compressedFile))
      throw_up("Compressed file '", compressedFile.wstr(), "' is already in the backup folder '", BACKUP_FOLDER.wstr(), "'");

    compressedFile.move_self_into(BACKUP_FOLDER);
    if (!BACKUP_FOLDER.has(compressedFile))
      throw_up("Failed to move compressed file '", compressedFile.wstr(), "' to backup folder '", BACKUP_FOLDER.wstr(), "'");
    out << L"Archived compressed file '" << compressedFile.wstr() << L"' to '" << BACKUP_FOLDER.wstr() << L"'";
  }


  void peek_into_compressed(const File& compressedFile) {
    /*
      Print the content of the compressed file
      to the console
      Example:
        peek_into_compressed(File(L"./docs.tar.lrz"));
        // Prints the content of the compressed file
    */
    if (!WORKING_DIR.has(compressedFile))
      throw_up("Compressed file '", compressedFile.wstr(), "' isn't in the working directory '", WORKING_DIR.wstr(), "'");
    
    out << "Content of '" << compressedFile.wstr() << "':\n";
    sh_call(LRZCAT_CMD_HEAD + to_str(compressedFile.wstr()) + LRZCAT_CMD_TAIL);
  }


  void pull_and_archive(const Folder& toBeCompressed) {
    /*
      Pull the folder from the exchange folder,
      compress it and archive it
      Example:
        pull_and_archive(Folder(L"./docs"));
        // Pulls the folder "docs" from "/root/shared", compresses it and archives it
    */
    out << "Pulling folder '" << toBeCompressed.wstr() << "' from exchange folder '" << EXCHANGE_FOLDER.wstr() << "'\n";
    Folder copyOfToBeCompressed = carry_here(toBeCompressed);
    File compressedFile = compress(copyOfToBeCompressed);
    archive(compressedFile);
    out << "Done! Compressed file is '" << compressedFile.wstr() << "'\n";
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
    out << "Next run will be on " << nextRun.as_wstr() << "\n";
    // Hibernate the program until the next run
    std::this_thread::sleep_until(nextRun.timePoint);
  }
};