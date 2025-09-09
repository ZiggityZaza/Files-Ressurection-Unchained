#include "../CeggsyLib/cslib.h++" // Adjust path as needed (get it on Github ZiggityZaza/CeggsyLib)
using namespace cslib;




namespace IncrementalBackup {
  /*
    Functionality to handle differences between
    two folders (original and current)
    Note:
      This namespace is MARVELOUS!
    Another note:
      All the functions are dog slow because they
      read all files fully into memory to compare
      (vector<char> comparison).
  */
  enum BackupChanges : char {
    MODIFIED = 'M',
    ADDED = 'A',
    DELETED = 'D'
  };


  using notepad_t = std::vector<std::pair<Road, BackupChanges>>;
  void note_deleted_raw(notepad_t& _notepad, Folder _original, Folder _current) noexcept {
    /*
      Note which files/folders are missing
      Example:
        note_deleted_raw(changes, "./original", "./current");
        changes == {
          {"folder/subfolder", DELETED},
          ...
        }
    */
    Out out = Out(std::cout, "[FRU IncrementalBackup note_deleted_raw]:", Red);
    for (Road entry : _original.untyped_list()) {
      out << "Entry: " << entry << std::endl;
      if (entry.type() == stdfs::file_type::regular)
        if (!_current.untyped_find(entry))
          _notepad.push_back({entry, DELETED});

      if (entry.type() == stdfs::file_type::directory) {
        maybe<Folder> subFolder(_current.untyped_find(entry));
        if (subFolder.has_value())
          note_deleted_raw(_notepad, Folder(*subFolder), Folder(entry));
        else
          _notepad.push_back({entry, DELETED});
      }

      if (entry.type() != stdfs::file_type::regular and entry.type() != stdfs::file_type::directory)
        exit_because(__LINE__, "Bizarre file types such as ", entry, " aren't supported yet.");
    }
  }


  void note_added_raw(notepad_t& _notepad, Folder _original, Folder _current) noexcept {
    /*
      Note which files/folders are new
      Example:
        note_added_raw(changes, "./original", "./current");
        changes == {
          {"folder/subfolder/newfile.txt", ADDED},
          ...
        }
    */
    Out out = Out(std::cout, "[FRU IncrementalBackup note_added_raw]:", Yellow);
    for (Road entry : _current.untyped_list()) {
      out << "Entry: " << entry << std::endl;
      if (entry.type() == stdfs::file_type::regular)
        if (!_original.untyped_find(entry))
          _notepad.push_back({entry, ADDED});

      if (entry.type() == stdfs::file_type::directory) {
        maybe<Folder> subFolder(_original.untyped_find(entry));
        if (subFolder.has_value())
          note_added_raw(_notepad, Folder(*subFolder), Folder(entry));
        else
          _notepad.push_back({entry, ADDED});
      }

      if (entry.type() != stdfs::file_type::regular and entry.type() != stdfs::file_type::directory)
        exit_because(__LINE__, "Bizarre file types such as ", entry, " aren't supported yet.");
    }
  }



  void note_changed_raw(notepad_t& _notepad, Folder _original, Folder _current) noexcept {
    /*
      Note which files/folders have been modified
      Example:
        note_changed_raw(changes, "./original", "./current");
        changes == {
          {"folder/subfolder/changedfile.txt", MODIFIED},
          ...
        }
    */
    Out out = Out(std::cout, "[FRU IncrementalBackup note_changed_raw]:", Green);
    for (Road entry : _current.untyped_list()) {
      out << "Entry: " << entry << std::endl;
      if (entry.type() == stdfs::file_type::regular) {
        maybe<File> originalFile(_original.untyped_find(entry));
        if (originalFile.has_value())
          if (File(entry).read_binary() != File(*originalFile).read_binary())
            _notepad.push_back({entry, MODIFIED});
      }

      if (entry.type() == stdfs::file_type::directory) {
        maybe<Folder> subFolder(_original.untyped_find(entry));
        if (subFolder.has_value())
          note_changed_raw(_notepad, Folder(*subFolder), Folder(entry));
      }

      if (entry.type() != stdfs::file_type::regular and entry.type() != stdfs::file_type::directory)
        exit_because(__LINE__, "Bizarre file types such as ", entry, " aren't supported yet.");
    }
  }




  void test() noexcept {
    /*
      Test the IncrementalBackup namespace
    */
    Out out = Out(std::cout, "[FRU IncrementalBackup Test]:", Magenta);
    Folder original("../original", true);
    Folder current("../current", true);

    notepad_t changes;
    note_deleted_raw(changes, original, current);
    note_added_raw(changes, original, current);
    note_changed_raw(changes, original, current);

    out << "Changes between " << original << " and " << current << ":\n";
    for (const auto& [entry, change] : changes) {
      str_t changeStr;
      switch (change) {
        case MODIFIED: changeStr = "Modified"; break;
        case ADDED: changeStr = "Added"; break;
        case DELETED: changeStr = "Deleted"; break;
        default: changeStr = "Unknown"; break;
      }
      out << "- " << changeStr << ": " << entry << '\n';
    }
    out << "Total changes: " << changes.size() << std::endl;
  }
};









int main() {
  while (true) {
    IncrementalBackup::test();
    std::cout << "Press Enter to exit...\n";
    std::cin.get();
  }
  // while (false) {
  //   // Step 1: Get all folders in the exchange folder
  //   std::vector<Folder> folders = LRZ::get_all_folders_in_exchange();

  //   for (const Folder& folder : folders)
  //     LRZ::pull_and_archive(folder);

  //   LRZ::go_back_to_sleep(); // Hibernate
  // }
}