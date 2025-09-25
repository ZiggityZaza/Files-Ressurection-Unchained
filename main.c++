#include "../CeggsyLib/cslib.h++" // Adjust path as needed (get it on Github ZiggityZaza/CeggsyLib)
#include <iostream>


using namespace cslib;
template <typename T>
using opt = std::optional<T>;
#define make_out(color) Out out = Out(std::cerr, "[" + to_str(__func__) + "]", color);




namespace IncrementalBackup {
  /*
    Functionality to handle differences between
    two folders (original and current)
    Note:
      This namespace is MARVELOUS!
  */
  enum BackupChanges : char {
    MODIFIED = 'M',
    ADDED = 'A',
    DELETED = 'D'
  };



  using notepad_t = std::vector<std::pair<Road, BackupChanges>>;
  void note_deleted_raw(notepad_t& notepad, Folder original, Folder current) {
    /*
      Note which files/folders are missing
      Example:
        note_deleted_raw(changes, "./original", "./current");
        changes == {
          {"folder/subfolder", DELETED},
          ...
        }
    */
    make_out(Red);

    for (Road ogEntry : original.list()) {
      opt<Road> counterPart = current.has(ogEntry.name());
      if (counterPart.has_value()) // If counter part exists, skip
        continue;

      if (ogEntry.type() == stdfs::file_type::directory) {
        opt<Folder> subFolder(*counterPart);
        if (subFolder.has_value())
          note_deleted_raw(notepad, *subFolder, Folder(ogEntry));
        else {
          notepad.push_back({ogEntry, DELETED});
          out << ogEntry << " was deleted...\n";
        }
      }
      else // Files, sockets, ...
        if (!counterPart) {
          notepad.push_back({ogEntry, DELETED});
          out << ogEntry << " was deleted...\n";
        }
    }
  }



  void note_added_raw(notepad_t& notepad, Folder original, Folder current) noexcept {
    /*
      Note which files/folders are new
      Example:
        note_added_raw(changes, "./original", "./current");
        changes == {
          {"folder/subfolder/newfile.txt", ADDED},
          ...
        }
    */
    make_out(Yellow);

    for (Road entry : current.list()) {
      if (entry.type() == stdfs::file_type::directory) {
        opt<Folder> subFolder(original.has(entry.name()));
        if (subFolder.has_value())
          note_added_raw(notepad, *subFolder, Folder(entry));
        else {
          notepad.push_back({entry, ADDED});
          out << entry << " is new...\n";
        }
      }
      else  // Files, sockets, ...
        if (!original.has(entry.name())) {
          notepad.push_back({entry, ADDED});
          out << entry << " is new...\n";
        }
    }
  }



  void note_changed_raw(notepad_t& notepad, Folder original, Folder current) noexcept {
    /*
      Note which files/folders have been modified
      Example:
        note_changed_raw(changes, "./original", "./current");
        changes == {
          {"folder/subfolder/changedfile.txt", MODIFIED},
          ...
        }
    */
    make_out(Green);

    for (Road entry : current.list()) {
      opt<Road> counterPart = original.has(entry.name());
      if (!counterPart.has_value())
        continue;
      else if (counterPart.has_value() && (*counterPart).type() != entry.type()) {
        notepad.push_back({entry, ADDED});
        notepad.push_back({*counterPart, DELETED});
        out << entry.name() << " was turned into another type...\n";
      }
      else if (entry.type() == stdfs::file_type::directory)
        note_changed_raw(notepad, Folder(*counterPart), Folder(entry));
      else if (entry.type() == stdfs::file_type::regular) {
        std::ifstream ogStream(counterPart.value().isAt, std::ios::binary);
        std::ifstream entryStream(entry.isAt, std::ios::binary);
        if (read_data(ogStream) != read_data(entryStream)) {
          notepad.push_back({entry, MODIFIED});
          out << entry.name() << " was modified...\n";
        }
      }
      else
        out << "Skipping " << entry << " because it is a special file and we don't handle those...\n";
    }
  }




  void test() noexcept {
    /*
      Test the IncrementalBackup namespace
    */
    make_out(Magenta);
    Folder original("C:\\Users\\BLECHBUCHSE\\Desktop\\original");
    Folder current("C:\\Users\\BLECHBUCHSE\\Desktop\\current");

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