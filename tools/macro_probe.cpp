#include <iostream>
#include "mactab.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "usage: macro_probe <in-file> <out-file>\n";
    return 2;
  }

  CMacroTable table;
  table.init();

  if (!table.loadFromFile(argv[1])) {
    std::cerr << "load_failed\n";
    return 1;
  }

  std::cout << "loaded_count=" << table.getCount() << "\n";

  if (!table.writeToFile(argv[2])) {
    std::cerr << "write_failed\n";
    return 1;
  }

  table.resetContent();

  if (!table.loadFromFile(argv[2])) {
    std::cerr << "reload_failed\n";
    return 1;
  }

  std::cout << "reloaded_count=" << table.getCount() << "\n";
  return 0;
}
