#include "resources/cslib.hpp" // Contains all libs


namespace FRU {
  // Project specific resources

  cslib::DualOutput out("log.txt");
  static std::string settingsPath = "settings.json";
  static uint8_t JSON_INDENT = 2;
  #define INITIAL_SETTINGS_CONTENT "{\n  \"BACKUP_INTO_PATH_USERS_PUBLIC\": \"C:\\Users\\Public\\Documents\\Backup\",\n  \"BACKUPED_FOLDERS\": []\n}"
  /*
    Note for how settings.json file looks like:
    {
      "BACKUP_INTO_PATH_USERS_PUBLIC": "C:\\Users\\Public\\Documents\\Backup",
      "BACKUPED_FOLDERS": [
        [FILE_PATH_AS_STR, -1(-1 Means at startup. Anything else aint implemented yet)]
      ]
    }
  */
  #define BCKPD_FLDR_PTH_INDX 0
  #define BCKPD_FLDR_INTRVL_INDX 1
  struct BackupFolder {
    cslib::VirtualPath where;
    // int interval; (Not implemented yet)
  };


  class Main { public:
    // Runtime resources

    nlohmann::json settings;
    std::deque<BackupFolder> backupFolders;


    void prase_settings() {
      if (!std::filesystem::exists(settingsPath)) {
        out << "No settings file found. If you deleted it, you are an idiot.\n";
        out << "'auto' - create one in here; 'C:\\...' - create one in the specified path\n";
        std::string path = out.ask();
        if (path != "auto")
          settingsPath = path;
        std::ofstream(settingsPath) << INITIAL_SETTINGS_CONTENT; // Create a new settings file
        out << "Settings file created\n";
      }
      std::ifstream(settingsPath) >> settings;
      out << "Settings file loaded\n";
      for (auto &folder : settings["BACKUPED_FOLDERS"]) {
        backupFolders.push_back({folder[BCKPD_FLDR_PTH_INDX].get<std::string>()});
      }
      out << "Settings parsed\n";
    }


    void startup_backup() {
      for (FRU::BackupFolder &folder : backupFolders) {
        // Incremental copy of files using built-in Windows robocopy
        std::string command = "robocopy " + folder.where.path + " " + settings["BACKUP_INTO_PATH_USERS_PUBLIC"].get<std::string>() + " /MIR /MT:4"; // std::thread::hardware_concurrency();
        system(command.c_str());
      }
    }



    Main() {
      // Setup and runtime
    }

  };
}


int main() {FRU::Main main;}