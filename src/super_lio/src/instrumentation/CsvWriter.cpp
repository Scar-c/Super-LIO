#include "instrumentation/CsvWriter.h"

#include <sstream>

namespace LI2Sup {

bool CsvWriter::open(const std::string& path,
                     const std::vector<std::string>& header) {
  file_.open(path, std::ios::out | std::ios::trunc);
  if (!file_.is_open()) return false;
  std::ostringstream oss;
  for (std::size_t i = 0; i < header.size(); ++i) {
    if (i) oss << ',';
    oss << header[i];
  }
  oss << '\n';
  file_ << oss.str();
  file_.flush();
  return true;
}

void CsvWriter::writeRow(const std::vector<std::string>& fields) {
  std::ostringstream oss;
  for (std::size_t i = 0; i < fields.size(); ++i) {
    if (i) oss << ',';
    oss << fields[i];
  }
  oss << '\n';
  file_ << oss.str();
  file_.flush();
}

void CsvWriter::flush() {
  if (!file_.is_open()) return;
  file_.flush();
}

void CsvWriter::close() {
  if (!file_.is_open()) return;
  flush();
  file_.close();
}

}  // namespace LI2Sup