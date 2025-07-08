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
  Folder decompress() = delete;
  // No support for decompression due to excessive complexity

  MACRO LRZCAT_CMD_HEAD = "lrzcat \"";
  MACRO LRZCAT_CMD_TAIL = "\" | tar -tv";
  MACRO COMPRESS_CMD_HEAD = "lrztar -z \"";
  MACRO COMPRESS_CMD_TAIL = "\""; // Keep it a string literal, so it can be concatenated with the path
  MACRO EXPECTED_EXTENSION = L".tar.lrz";
  std::wostream& out = std::wcout;
  const Folder EXCHANGE_FOLDER(L"/root/shared"); // Grab targets from (immutable)
  Folder BACKUP_FOLDER(L"/root/archive"); // Put compressed files in here
  Folder WORKING_DIR(std::filesystem::current_path().wstring());



  Folder carry_here(const Folder& toBeCompressed) {
    /*
      Copy the folder that is supposed to be
      handled by this program to this directory
      Note:
        Target MUST be in the Exchange Folder
      Example:
        carry_here(Folder(L"/root/shared/docs"));
        // Copies the folder "docs" to "/root/shared"
    */
    if (toBeCompressed.is.parent().isAt != EXCHANGE_FOLDER.is.isAt)
      throw_up("Target folder '", toBeCompressed.wstr(), "' isn't in the exchange folder '", EXCHANGE_FOLDER.wstr(), "'"); 
    if (contains(WORKING_DIR.content, toBeCompressed.is))
      throw_up("Target folder '", toBeCompressed.wstr(), "' is already in the working directory '", WORKING_DIR.wstr(), "'");

    out << "Copying folder '" << toBeCompressed.wstr() << "' to working directory '" << WORKING_DIR.wstr() << "'";
    Folder copyOfToBeCompressed(toBeCompressed.is.copy_into(WORKING_DIR.is).isAt.wstring());
    WORKING_DIR.update();
    out << "Copied folder '" << toBeCompressed.wstr() << "' to working directory '" << WORKING_DIR.wstr() << "'\n";
    if (!contains(WORKING_DIR.content, copyOfToBeCompressed.is))
      throw_up("Failed to copy folder '", toBeCompressed.wstr(), "' to working directory '", WORKING_DIR.wstr(), "'");
    return copyOfToBeCompressed;
  }


  void archive(File& compressedFile) {
    /*
      Move the compressed file to the backup
      folder
      Example:
        File compressedFile(L"./docs.tar.lrz");
        archive(compressedFile);
        // Moves the file to "/root/archive/docs.tar.lrz"
    */
    if (compressedFile.is.parent().isAt != WORKING_DIR.is.isAt)
      throw_up("Compressed file '", compressedFile.wstr(), "' isn't in the working directory '", WORKING_DIR.wstr(), "'");
    if (contains(BACKUP_FOLDER.content, compressedFile.is))
      throw_up("Compressed file '", compressedFile.wstr(), "' is already in the backup folder '", BACKUP_FOLDER.wstr(), "'");

    compressedFile.is.move_to(BACKUP_FOLDER.is);
    BACKUP_FOLDER.update();
    if (!contains(BACKUP_FOLDER.content, compressedFile.is))
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
    if (compressedFile.is.parent().isAt != WORKING_DIR.is.isAt)
      throw_up("Compressed file '", compressedFile.wstr(), "' isn't in the working directory '", WORKING_DIR.wstr(), "'");
    
    out << "Content of '" << compressedFile.wstr() << "':\n";
    sh_call(LRZCAT_CMD_HEAD + to_str(compressedFile.wstr()) + LRZCAT_CMD_TAIL);
  }


  File compress(const Folder& toBeCompressed) {
    /*
      Compress the folder and return the compressed file
      Example:
        File compressedFile = compress(Folder(L"./docs"));
        // Compressed file is "./docs.tar.lrz"
    */
    if (toBeCompressed.is.parent().isAt != WORKING_DIR.is.isAt)
      throw_up("Folder '", toBeCompressed.wstr(), "' isn't in the working directory '", WORKING_DIR.wstr(), "'");
    
    wstr_t willBecome = toBeCompressed.is.isAt.wstring() + EXPECTED_EXTENSION;
    if (std::filesystem::exists(willBecome))
      throw_up("Compressed file '", willBecome, "' already exists in the working directory '", WORKING_DIR.wstr(), "'");

    out << "Compressing folder '" << toBeCompressed.wstr() << "' to '" << willBecome << "'";
    sh_call(COMPRESS_CMD_HEAD + to_str(toBeCompressed.wstr()) + COMPRESS_CMD_TAIL);
    out << "Compressed folder '" << toBeCompressed.wstr() << "' to '" << willBecome << "'\n";
    WORKING_DIR.update();
    File willBecomeFile(willBecome);
    if (!contains(WORKING_DIR.content, willBecomeFile.is))
      throw_up("Failed to compress folder '", toBeCompressed.wstr(), "' to '", willBecome, "'");
    return willBecomeFile;
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
      month at 3:30 AM
      Example:
        go_back_to_sleep();
        // Hibernates the program until the next run
    */
    out << "Going back to sleep...\n";
    
    // Get the current time and calculate the next run time
    using namespace std::chrono;

    auto now = std::chrono::system_clock::now();
    auto today = std::chrono::floor<std::chrono::days>(now); // Midnight today
    std::chrono::year_month_day todayYMD = std::chrono::year_month_day{today};

    std::chrono::year_month_day next_month
  }
};