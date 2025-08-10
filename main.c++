#include "resources/lrzip.h++"

int main() {
  using namespace LRZTAR;
  while(true) {
    // Step 1: Get all folders in the exchange folder
    std::vector<Folder> folders = get_all_folders_in_exchange();

    for (const Folder& folder : folders)
      pull_and_archive(folder);

    go_back_to_sleep(); // Hibernate
  }
}