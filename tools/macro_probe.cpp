//------------------------------------------------------------------------------
// UniKey Vietnamese Input Method
// Copyright (C) 2000-2005 Pham Kim Long
// Copyright (C) 2026 Hoang Duy Tran & AI Agent
// Contact:
//   unikey@gmail.com
//   hoangduytran1960@googlemail.com
//   UniKey project: http://unikey.org
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//------------------------------------------------------------------------------

// simple utility used during development/testing of CMacroTable
// extracts count from a macro file and writes a copy to another location.

#include <iostream>
#include "mactab.h"

/**
 * @file macro_probe.cpp
 * @brief Development helper to verify CMacroTable load/write cycles.
 *
 * Usage: `macro_probe <in-file> <out-file>`
 *
 * Loads the specified input macro file, reports the entry count, writes it to
 * the output file, then reloads the output file and reports the count again.
 * This helps detect serialization issues during changes to CMacroTable.
 * @param argc Number of command-line arguments (should be 3).
 * @param argv Array of argument strings: input file path followed by output file path.
 * @return Exit status: 0 on success, non-zero on error.
 */

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
