/**
 * Author: Declan Atkins
 * Last Modified: 19/05/19
 * 
 * Header file for the dataset class
 * 
 * Also contains functions for cross platform file loading
 */

#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include <cmath>
#include "document.h"

struct dataset_exception : std::runtime_error{
    dataset_exception(const std::string& message) : std::runtime_error(message){}
};


#if __cplusplus > 201402L

    #include <filesystem>

    std::vector<std::string> CPP2017_list_directory(std::string directory_name){
        std::vector<std::string> files;
        for (auto& file_or_dir : fs::recursive_directory_iterator(directory_name)){
            if (file_or_dir.is_regular_file()){
                files.push_back(file_or_dir.path());
            }
        }

        return files;
    }

    #define list_directory CPP2017_list_directory


#elif defined(_WIN32)

    #include <Windows.h>
    #include <iostream>
    #include <string>

    std::vector<std::string> WINDOWS_list_directory(std::string directory_name){
        std::string mod_dir = directory_name;
        std::vector<std::string> files;
        if (directory_name.back() != '/' && directory_name.back() != '\\'){
            mod_dir += "/*";
        }
        else{
            directory_name.pop_back();
            mod_dir = directory_name + "/*";
        }
        WIN32_FIND_DATA data;
        HANDLE hFind = FindFirstFile(mod_dir.c_str(), &data);

        if ( hFind != INVALID_HANDLE_VALUE ) {
            do {
                const char* ignore_dir1 = ".";
                const char* ignore_dir2 = "..";
                int not_ignore1 = strcmp(ignore_dir1, data.cFileName);
                int not_ignore2 = strcmp(ignore_dir2, data.cFileName);
                if (not_ignore1 && not_ignore2){
                    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                        std::string sub_dir = directory_name + '/' + data.cFileName;
                        std::vector<std::string> sub_files = WINDOWS_list_directory(sub_dir);
                        files.reserve(files.size() + distance(sub_files.begin(), sub_files.end()));
                        files.insert(files.end(), sub_files.begin(), sub_files.end());
                    }
                    else{
                        std::string f_name = directory_name + '/' + data.cFileName;
                        files.push_back(f_name);
                    }
                }
            } while (FindNextFile(hFind, &data));
            FindClose(hFind);
        }
        return files;
    }

    #define list_directory WINDOWS_list_directory


#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>

    std::vector<std::string> list_dir(const std::string dir){
        
        DIR *dp;
        struct dirent *dirp;
        std::vector<std::string> files;

        if ((dp = opendir(dir.c_str())) == NULL){
            throw dataset_exception("Could not open the directory")
        }

        while ((dirp = readdir(dp)) != NULL){
            std::string entry = dirp->d_name;
            if ( entry == "." or entry == ".." )
            {
                continue;
            }

            files.push_back(entry);
        }

        closedir(dp);
        return files;
    }
    
    std::vector<std::string> UNIX_list_directory(std::string directory_name){
        std::vector<std::string> files;
        std::vector<std::string> files_or_dirs = list_dir(directory_name);
        struct stat sb;
        

        for (auto it = files_or_dirs.begin(); it != files_or_dirs.end() ; it++){
            std::string entry = *it;
            std::string full_path = directory_name + "/" + entry;

            if (lstat(full_path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)){
                std::vector<std::string> sub_files = UNIX_list_directory(full_path);
                files.reserve(files.size() + distance(sub_files.begin(), sub_files.end()));
                files.insert(files.end(), sub_files.begin(), sub_files.end());

            } 
            else {
                files.push_back(full_path);
            }
        }

        return files;
    }

    #define list_directory UNIX_list_directory

#endif // OS CHECK


enum dataset_type_t{
    BINARY_MODEL,
    TEXT_DATASET,
    ZIPFILE
};


class Dataset{
    std::string root_dir;
    std::vector<Document> documents;
    std::map<std::wstring, double> idf;
    void compute_idf();
    void compute_tf_idf_for_documents();
public:
    Dataset(std::string, dataset_type_t, int);
    void output_keywords(int);
};


#endif // DATASET_H