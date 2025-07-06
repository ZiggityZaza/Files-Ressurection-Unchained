#include "resources/cslib.h++"
#pragma once


namespace LRZTAR { // TODO: Check https://github.com/ckolivas/lrzip for lrzdir
  using namespace cslib;
  /*
    Compress and decompress stuff using LRZip
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
      as the binary and the compressed file.
  */

  MACRO LRZTAR_VIEW_CLI = L"lrzcat \"{FILEPATH}\" | tar -tv";
  MACRO LRZTAR_COMPRESS_CLI = L"lrztar -z ";
  MACRO LRZTAR_DECOMPRESS_CLI = L"lrztar -d ";
  MACRO LRZTAR_EXTENSION = L".tar.lrz";
  Out lrztarOut("[LRZTAR]:");


  void print_lrztar_content(File sourceFile) {
    /*
      Print the content of a tar.lrz file.
      Example:
        print_lrztar_content("my_dir.tar.lrz");
        // Prints the content of my_dir.tar.lrz
    */

    // std::string command = format(LRZTAR_VIEW_CLI, {{"FILEPATH", sourceFile.asStr()}}); // lrzcat "my_dir.tar.lrz" | tar -tv
    cslib::sh_call(command);
  }


  File compress(Folder sourceDir) {
    /*
      Compress a directory into a tar.lrz file. A tar.lrz file will be
      created next the executable. The sourceDir will not be deleted.
      Example:
        File compressed = compress("docs");
        // docs.tar.lrz is created in workspace
      Note: If sourceDir is deleted, instances of it will be invalid
      and cause a crash if accessed.
    */

    if (sourceDir.location.parent_path().locationAsStr != std::filesystem::current_path().string())
      throw std::runtime_error("Source dir '" + sourceDir.asStr() + "' is not in working dir '" + std::filesystem::current_path().string() + "'");
    
    std::string willBecome = sourceDir.location.filename() + LRZTAR_EXTENSION; // docs + .tar.lrz
    if (std::filesystem::exists(willBecome))
      THROW_HERE("File '" + willBecome + "' already exists");

    cslib::sh_call(str(LRZTAR_COMPRESS_CLI) + " \"" + sourceDir.location.filename() + "\""); // lrztar -z docs

    return File(willBecome);
  }
  File compress(Folder sourceDir, Folder putIn) {
    /*
      Compress a directory into a tar.lrz file. The sourceDir will be deleted.
      Example:
        File compressed = compress_and_move("docs", "/root/backups");
        // docs.tar.lrz is created in workspace and moved to /root/backups
    */

    std::string wouldBecome = putIn.asStr() + VirtualPath::PATH_SEPARATOR + sourceDir.location.filename() + LRZTAR_EXTENSION;
    if (std::filesystem::exists(wouldBecome))
      THROW_HERE("File '" + wouldBecome + "' already exists");

    compress(sourceDir).location.move_to(putIn.location);

    return File(wouldBecome); // Check for existence again with constructor
  }


  Folder decompress(File sourceFile) {
    /*
      Decompress a tar.lrz file. Folder is located in workspace
      Example:
        Folder decompressed = decompress("docs.tar.lrz");
        // docs is created in workspace
    */

    if (sourceFile.location.parent_path().locationAsStr != std::filesystem::current_path().string())
      throw std::runtime_error("Source file '" + sourceFile.location.parent_path().locationAsStr + "' is not in working dir '" + std::filesystem::current_path().string() + "'");

    std::string sourceName = sourceFile.asStr(); // docs.tar.lrz
    if (sourceName.substr(sourceName.length() - str(LRZTAR_EXTENSION).length()) != LRZTAR_EXTENSION)
      THROW_HERE("File '" + sourceName + "' is not a '" + LRZTAR_EXTENSION + "' file");
      /*
        .extension() wouldn't recognize the .tar in .tar.lrz as part of the extension
      */

    std::string willBecome = sourceName.substr(0, sourceName.length() - str(LRZTAR_EXTENSION).length()); // Remove ".tar.lrz"
    if (std::filesystem::exists(willBecome))
      THROW_HERE("Folder '" + willBecome + "' already exists");

    cslib::sh_call(str(LRZTAR_DECOMPRESS_CLI) + " \"" + sourceName + "\""); // lrztar -d "docs.tar.lrz"
    return Folder(willBecome);
  }
  Folder decompress(File sourceFile, Folder putIn) {
    /*
      Decompress a tar.lrz file. The sourceFile will be deleted.
      Example:
        Folder decompressed = decompress_and_move("docs.tar.lrz", "/root/backups");
        // docs is created in workspace and moved to /root/backups
    */

    std::string willBecome = putIn.asStr() + VirtualPath::PATH_SEPARATOR + sourceFile.location.filename().substr(0, sourceFile.location.filename().length() - str(LRZTAR_EXTENSION).length());
    if (std::filesystem::exists(willBecome))
      THROW_HERE("Folder '" + willBecome + "' already exists");

    Folder ex = decompress(sourceFile);
    ex.location.move_to(putIn.location);
    return Folder(willBecome);
  }
};