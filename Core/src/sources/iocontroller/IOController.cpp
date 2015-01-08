/*
 * Copyright (c) 2014, webvariants GmbH, http://www.webvariants.de
 *
 * This file is released under the terms of the MIT license. You can find the
 * complete text in the attached LICENSE file or online at:
 *
 * http://www.opensource.org/licenses/mit-license.php
 *
 * @author: Thomas Krause (thomas.krause@webvariants.de)
 */

#include "iocontroller/IOController.h"

//Constructor
Susi::IOController::IOController() {}

//high level
std::size_t Susi::IOController::writeFile( std::string filename,char* ptr, std::size_t len ) {
    std::ofstream s( ( char* )(makeAbsolute(filename).c_str()) );
    s<<ptr;
    s.close();
    return len;
}

std::size_t Susi::IOController::writeFile( std::string filename,std::string content ) {
    return this->writeFile( filename, ( char* )content.c_str(), content.size() );
}

std::string Susi::IOController::readFile( std::string filename ) {

    if( this->checkFile( this->makeAbsolute( filename ) ) ) {
        std::string result = "";
        int length = 1024;
        std::ifstream s( ( char* )filename.c_str() );

        if( s ) {

            char buff[length];
            u_long bs = length;
            while( s.good() && bs > 0 ) {
                bs = s.readsome( buff,length );
                result += std::string {buff,bs};
            }

            s.close();

            return result;
        }

        throw std::runtime_error {"ReadFile: can't read file!"};
    }
    else {
        std::string msg = "ReadFile: ";
        msg += filename;
        msg += " don't exists!";
        LOG(ERROR) <<  msg ;
        throw std::runtime_error {"ReadFile: File don't exists!"};
    }
}

bool Susi::IOController::movePath( std::string source_path, std::string dest_path ) {
    std::string sf( this->makeAbsolute( source_path ) );
    std::string dp = this->makeAbsolute( dest_path );
    std::string command = "mv "+sf+" "+dp;
    return system(command.c_str())==0;
}

bool Susi::IOController::copyPath( std::string source_path, std::string dest_path ) {
    std::string sf( this->makeAbsolute( source_path ) );
    std::string dp = this->makeAbsolute( dest_path );
    std::string command = "cp -rf "+sf+" "+dp;
    return system(command.c_str())==0;
}

bool Susi::IOController::deletePath( std::string path ) {
    std::string p( this->makeAbsolute( path ) );
    std::string command = "rm -rf "+p;
    return system(command.c_str())==0;
}

// low level
bool Susi::IOController::makeDir( std::string dir ) {
    std::string p( this->makeAbsolute( dir ) );
    std::string command = "mkdir -p "+p;
    return system(command.c_str())==0;
}

bool Susi::IOController::setExecutable( std::string path, bool isExecutable ) {
    std::string p( this->makeAbsolute( path ) );
    std::string command = "chmod +x "+p;
    return system(command.c_str())==0;
}

bool Susi::IOController::getExecutable( std::string path ) {
    std::string p( this->makeAbsolute( path ) );
    struct stat sb;
    if (stat(p.c_str(), &sb) == -1) {
        perror("stat");
        return false;
    }
    return sb.st_mode & S_IXUSR;
}

// helper function
std::string Susi::IOController::makeAbsolute(std::string path){
    char *abs = realpath(path.c_str(),NULL);
    std::string result{abs};
    free(abs);
    return result;
}

bool Susi::IOController::checkDir( std::string dir ) {
    std::ifstream s{dir};
    return s;
}

bool Susi::IOController::checkFile( std::string dir ) {
    std::ifstream s{dir};
    return s;
}

bool Susi::IOController::checkFileExtension( std::string path, std::string file_extension ) {

    if( path.length() >= file_extension.length() + 1 ) {
        std::string sub_str = path.substr( path.length() - file_extension.length() - 1 );

        return ( sub_str == ( "." + file_extension ) );
    }

    return false;
}
