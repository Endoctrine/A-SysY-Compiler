//
// Created by Edot on 2023/9/18.
//

#include "FileReader.h"

FileReader::FileReader(const char *path): buffer(0), eof_putback(-1){
    file_stream = new std::ifstream(path);
    if(!file_stream->is_open()){
        perror("FileReader");
        exit(-1);
    }
}

FileReader::~FileReader(){
    file_stream->close();
    delete file_stream;
}

bool FileReader::GetChar(char &c){
    if(eof_putback >= 0){
        if(eof_putback){
            eof_putback--;
            c = buffer;
            return true;
        } else{
            eof_putback--;
            return false;
        }
    }
    if(file_stream->get(c)){
        buffer = c;
        return true;
    } else {
        return false;
    }
}

void FileReader::PutBack(){
    if(!file_stream->unget()){
        if(eof_putback >= 1){
            perror("FileReader::PutBack: buffer overflow.");
        }
        eof_putback++;
    }
}
