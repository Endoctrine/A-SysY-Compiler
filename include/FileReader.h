//
// Created by Edot on 2023/9/18.
//

#ifndef EXP2_CPP_FILE_READER_H_
#define EXP2_CPP_FILE_READER_H_
#include <fstream>
#include <vector>

class FileReader{
  public:
    explicit FileReader(const char *path);
    ~FileReader();
    bool GetChar(char &c);
    void PutBack();

  private:
    std::ifstream *file_stream;
    char buffer;
    int eof_putback;
};

#endif //EXP2_CPP_FILE_READER_H_
