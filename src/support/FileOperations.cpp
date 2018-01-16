#include "FileOperations.h"

using namespace std;

FileOperations::FileOperations()
{
}

vector<shared_ptr<Argument>> FileOperations::getInputArguments() const
{
    return inputArguments_;
}

vector<string> FileOperations::getFlowFilesName() const
{
    return flowFilesName_;
}

string FileOperations::getFileExtension(const string& filename) const
{
    string extension = "";

    for (int i = 0; i < filename.size(); ++i)
    {
        if (filename[i] == '.')
        {
            for (int j = i; j < filename.size(); ++j)
            {
                extension.push_back(filename[j]);
            }
        }
    }

    return extension;
}

int FileOperations::getFileNumber(const std::string& filename) const
{
    string number = "";

    for (int i = 0; i < filename.size(); ++i)
    {
        if (isdigit(filename[i]))
            number.push_back(filename[i]);
    }

    return stoi(number);
}

string FileOperations::intToString(const int number)
{
    string numberStr = to_string(number);

    int lengthStr = static_cast<int>(numberStr.size());
    int padding = LENGTH_STR_FRAMES - lengthStr;

    string str = "";

    for (int i = 0; i < padding; ++i)
        str.push_back('0');

    return str + numberStr;
}

void FileOperations::parseInputFile(const std::string& filename)
{
    ifstream file;
    file.open(filename);

    if (!file.is_open())
    {
        cerr << "Unable to open the file: " + filename << endl;
        return;
    }

    string inputLine;

    while (getline(file, inputLine, '\n'))
    {
        vector<string> parsedLine = parseInputLine(inputLine);

        shared_ptr<Argument> lineArgument (new Argument());

        lineArgument->setPathProgram(parsedLine[PATH_PROGRAM]);
        lineArgument->setProgramName(parsedLine[PROGRAM]);
        lineArgument->setPathImage1(parsedLine[IMAGE1]);
        lineArgument->setPathImage2(parsedLine[IMAGE2]);
        lineArgument->setOutName(parsedLine[OUTPUT_NAME]);
        lineArgument->setPathBackgroundImage(parsedLine[BACKGROUND]);

        inputArguments_.push_back(lineArgument);
    }
}

vector<string> FileOperations::parseInputLine(const std::string& line)
{
    vector<string> parsedLine;

    string word = "";

    for (int i = 0; i < line.size(); ++i)
    {
        if (line[i] != ' ')
        {
            word.push_back(line[i]);
        }
        else
        {
            parsedLine.push_back(word);
            word = "";
        }
    }

    parsedLine.push_back(word);

    return parsedLine;
}

void FileOperations::retrieveFlowFiles(const std::string& directory, const int nbFrames)
{
    DIR* directoryPath;
    struct dirent* currentDirectory;

    directoryPath = opendir(directory.c_str());

    if (directoryPath== NULL)
    {
        cerr << "Error opening: " << directory << endl;
        return;
    }

    flowFilesName_.reserve(static_cast<unsigned>(nbFrames));

    currentDirectory = readdir(directoryPath);

    while (currentDirectory != NULL)
    {
        string filename = static_cast<string>(currentDirectory->d_name);
        string extension = getFileExtension(filename);

        if (extension == EXTENSION_FLO)
            flowFilesName_.push_back(filename);

        currentDirectory = readdir(directoryPath);
    }

    closedir(directoryPath);

    sortFlowFiles();
}

void FileOperations::sortFlowFiles()
{
    vector<string> original = flowFilesName_;

    string nextFileToSort;

    for (int i = 0; i < flowFilesName_.size(); ++i)
    {
        for (int j = 0; j < original.size(); ++j)
        {
            int number = getFileNumber(original[j]);
            if (number == i + 1)
            {
                nextFileToSort= original[j];
                break;
            }
        }
        flowFilesName_[i] = nextFileToSort;
    }
}
