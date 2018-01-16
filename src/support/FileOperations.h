#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <dirent.h>

#include "Argument.h"

#ifndef IMOT_OPTICALFLOW_EDGES_FILEOPERATIONS_H
#define IMOT_OPTICALFLOW_EDGES_FILEOPERATIONS_H

const int LENGTH_STR_FRAMES = 8;

const std::string EXTENSION_FLO = ".flo";
const std::string EXTENSION_JPG = ".jpg";
const std::string EXTENSION_PNG = ".png";

const std::string RELATIVE_PATH = "../dataset/";

class FileOperations
{
public:

    FileOperations();

    std::vector<std::shared_ptr<Argument>> getInputArguments() const;
    std::vector<std::string> getFlowFilesName() const;
    std::string getFileExtension(const std::string& filename) const;
    int getFileNumber(const std::string& filename) const;
    std::string intToString(const int number);

    void parseInputFile(const std::string& filename);
    std::vector<std::string> parseInputLine(const std::string& line);

    void retrieveFlowFiles(const std::string& directory, const int nbFrames);
    void sortFlowFiles();

private:

    enum ArgumentsPosition {PATH_PROGRAM = 0, PROGRAM = 1, IMAGE1 = 2, IMAGE2 = 3, OUTPUT_NAME = 4, BACKGROUND = 5};

    std::vector<std::shared_ptr<Argument>> inputArguments_;
    std::vector<std::string> flowFilesName_;
};

#endif //IMOT_OPTICALFLOW_EDGES_FILEOPERATIONS_H
