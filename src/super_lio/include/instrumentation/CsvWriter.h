#ifndef CSV_WRITER_H_
#define CSV_WRITER_H_

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

namespace LI2Sup {

class CsvWriter {
 public:
  CsvWriter() = default;
  ~CsvWriter() { close(); }

  bool open(const std::string& path, const std::vector<std::string>& header);
  void writeRow(const std::vector<std::string>& fields);
  void flush();
  void close();
  bool isOpen() const { return file_.is_open(); }

 private:
  std::ofstream file_;
};

}  // namespace LI2Sup

#endif  // CSV_WRITER_H_