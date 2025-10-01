#include "../CeggsyLib/cslib.h++" // Adjust path as needed (get it on Github ZiggityZaza/CeggsyLib)
#include <iostream>
using namespace cslib;
const Folder ORIGINAL("C:\\Users\\BLECHBUCHSE\\Desktop\\og");
const Folder CURRENT("C:\\Users\\BLECHBUCHSE\\Desktop\\new");

std::ostream& out = std::cerr;
// std::ofstream out("./logs.txt");

// BROKEN: Hybrid subfile change not detected





template <typename T>
using opt = std::optional<T>;




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



  // Handle writing down differences
  std::vector<std::pair<str_t, BackupChanges>> notepad;
  
  void was_deleted(Road original) {
    // "/.../orginal/f1" -> "original/f1"
    if (!original.str().starts_with(ORIGINAL.str()))
      throw std::invalid_argument("Originally deleted entry '" + original.str() + "' isn't in the original dir '" + ORIGINAL.str() + "'");
    notepad.push_back({original.str().substr(ORIGINAL.str().length() - ORIGINAL.name().length()), DELETED});
  }
  void was_modified(Road current) {
    // "/.../current/f1" -> "current/f1"
    if (!current.str().starts_with(CURRENT.str()))
      throw std::invalid_argument("Originally modified entry '" + current.str() + "' isn't in the original dir '" + CURRENT.str() + "'");
    notepad.push_back({current.str().substr(CURRENT.str().length() - CURRENT.name().length()), MODIFIED});
  }
  void was_added(Road current) {
    // "/.../current/f1" -> "current/f1"
    if (!current.str().starts_with(CURRENT.str()))
      throw std::invalid_argument("Currently added entry '" + current.str() + "' isn't in the current dir '" + CURRENT.str() + "'");
    notepad.push_back({current.str().substr(CURRENT.str().length() - CURRENT.name().length()), ADDED});
  }



  void note_deleted(Folder original, Folder current, size_t indent) noexcept {
    /*
      Note which files/folders are missing
      Example:
        note_deleted("./original", "./current");
        notepad == {
          {"original/folder", DELETED},
          ...
        }
    */
    for (Road originalEntry : original.list()) {
      opt<Road> currentEntry = current.has(originalEntry.name());
      out << '\n' << std::string(indent, ' ') << " > Original: " << originalEntry;

      if (originalEntry == stdfs::file_type::directory)
        if (currentEntry.has_value() && *currentEntry == stdfs::file_type::directory) {
          out << " >>> 📂⤵️";
          note_deleted(Folder(originalEntry), Folder(*currentEntry), indent + 1);
        }
        else {
          out << " >>> 📁🗑️";
          was_deleted(originalEntry);
        }

      else // Files, sockets, ...
        if (!currentEntry || originalEntry.type() != currentEntry.value().type()) {
          was_deleted(originalEntry);
          out << " >>> 📄🗑️";
        }
    }
  }



  void note_changed(Folder original, Folder current, size_t indent) noexcept {
    /*
      Note which files/folders have been modified
      Example:
        note_changed("./original", "./current");
        notepad == {
          {"changed/folder/changedfile.txt", MODIFIED},
          ...
        }
    */
    for (Road originalEntry : original.list()) {
      out << '\n' << std::string(indent, ' ') << " > Original: " << originalEntry;
      opt<Road> currentEntry = current.has(originalEntry.name());

      if (!currentEntry.has_value() || currentEntry.value().type() != originalEntry.type()) // Not handled here
        continue;
      
      if (originalEntry == stdfs::file_type::directory) {
        out << " >>> 📂⤵️";
        note_changed(Folder(originalEntry), Folder(*currentEntry), indent + 1);
      }

      else if (originalEntry == stdfs::file_type::regular) {
        std::ifstream original_ifstream(originalEntry.isAt, std::ios::binary);
        std::ifstream current_ifstream(currentEntry.value().isAt, std::ios::binary);
        if (read_data(original_ifstream) != read_data(current_ifstream)) {
          out << " >>> 📝";
          was_modified(*currentEntry);
        }
      }

      else {
        if (originalEntry.last_modified() != currentEntry.value().last_modified()) {
          out << " >>> 📄📅";
          was_modified(*currentEntry);
        }
      }
    }
  }



  void note_added(Folder original, Folder current, size_t indent) noexcept {
    /*
      Note which files/folders are new
      Example:
        note_added("./original", "./current");
        notepad == {
          {"current/newfile.txt", ADDED},
          ...
        }
    */
    for (Road currentEntry : current.list()) {
      opt<Road> originalEntry(original.has(currentEntry.name()));
      out << '\n' << std::string(indent, ' ') << " > Current: " << currentEntry;

      if (currentEntry == stdfs::file_type::directory) {
        if (originalEntry.has_value() && *originalEntry == stdfs::file_type::directory) {
          out << " >>> 📂⤵️";
          note_added(Folder(*originalEntry), Folder(currentEntry), indent + 1);
        }
        else {
          out << " >>> 📂➕... (including " << Folder(*currentEntry).list().size() << " subentries)";
          was_added(Folder(*currentEntry));
        }
      }
      else { // Files and such
        if (!originalEntry.has_value() || originalEntry.value().type() != currentEntry.type()) {
          out << " >>> 📄➕";
          was_added(currentEntry);
        }
      }
    }
  }



  void write_down_changes() {
    std::vector<std::pair<Road, BackupChanges>> result;
    TempFolder differences;
    File explainingFile(differences.str() + "\\@explainingFile.txt", true);
    std::ostringstream explained;
    out << "DELETED: \n";
    for (const auto& [relativePath, change] : notepad) {
      if (change != DELETED)
        continue;
      explained << relativePath << " >>> DELETED\n";
      result.push_back({Road::create_self(ORIGINAL.str().substr(0, ORIGINAL.str().find_last_of("/\\")) + "\\" + relativePath), DELETED});
      out << result.back().first << '\n';
    }
    out << "MODIFIED: \n";
    for (const auto& [relativePath, change] : notepad) {
      if (change != MODIFIED)
        continue;
      explained << relativePath << " >>> MODIFIED\n";
      result.push_back({Road::create_self(CURRENT.str().substr(0, CURRENT.str().find_last_of("/\\")) + "\\" + relativePath), MODIFIED});
      out << result.back().first << '\n';
      switch (result.back().first.type()) {
        case stdfs::file_type::directory:
          Folder(result.back().first).copy_self_into(differences);
          break;
        case stdfs::file_type::regular:
          File(result.back().first).copy_self_into(differences);
          break;
      }
    }
    out << "ADDED: \n";
    for (const auto& [relativePath, change] : notepad) {
      if (change != ADDED)
        continue;
      explained << relativePath << " >>> ADDED\n";
      result.push_back({Road::create_self(CURRENT.str().substr(0, CURRENT.str().find_last_of("/\\")) + "\\" + relativePath), ADDED});
      out << result.back().first << '\n';
      switch (result.back().first.type()) {
        case stdfs::file_type::directory:
          Folder(result.back().first).copy_self_into(differences);
          break;
        case stdfs::file_type::regular:
          File(result.back().first).copy_self_into(differences);
          break;
      }
    }

    out << "Oh and these are different from the og:\n";
    for (Road r : differences.list())
      out << r << '\n';
    explainingFile.edit_text(explained.str());
    out << "Waiting here for " << explainingFile << std::endl;
    return;
  }



  void slice_differences() {
    // Called at the end
    TempFolder changesList;
    std::ostringstream inputData;
    for (const auto& [relativePath, change] : notepad) {
      inputData << relativePath << " >>> ";
      switch (change) {
        case MODIFIED: inputData << "MODIFIED"; break;
        case ADDED: inputData << "ADDED"; break;
        case DELETED: inputData << "DELETED"; break;
        default: throw "impossible";
      }
    }
  }


  void test() {
    /*
      Test the IncrementalBackup namespace
    */

    out << '[' << TimeStamp().as_str() << "]" << Bold << Red << "[note_deleted_raw]" << Reset << ':';
    note_deleted(ORIGINAL, CURRENT, 1);
    note_changed(ORIGINAL, CURRENT, 1);
    note_added(ORIGINAL, CURRENT, 1);
    out << Reset << '\n' << std::string(100, '_') << '\n';

    out << "\nChanges between OG: " << ORIGINAL << " and NEW: " << CURRENT << ":\n";
    for (const auto& [entryAsStr, change] : notepad) {
      str_t changeStr;
      switch (change) {
        case MODIFIED: changeStr = "✏️ "; break;
        case ADDED: changeStr = "➕ "; break;
        case DELETED: changeStr = "🗑️ "; break;
        default: throw "impossible";
      }
      out << " * " << entryAsStr << " --> " << changeStr << '\n';
    }
    out << Bold << "\nTotal changes: " << notepad.size() << Reset << std::endl;

    out << "Something else:\n";
    write_down_changes();
    out << "Not done yet!";
  }
};




int main() {
  while (true) {
    IncrementalBackup::notepad.clear();
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