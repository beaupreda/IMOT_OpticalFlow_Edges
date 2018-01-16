#include "Argument.h"

using namespace std;

Argument::Argument()
{
}

Argument::Argument(std::string pathProgram, std::string programName, std::string pathImage1,
                   std::string pathImage2, std::string outName, std::string pathBackgroundImage)
{
    pathProgram_ = pathProgram;
    programName_ = programName;
    pathImage1_ = pathImage1;
    pathImage2_ = pathImage2;
    outName_ = outName;
    pathBackgroundImage_ = pathBackgroundImage;
}

string Argument::getPathProgram() const
{
    return pathProgram_;
}

string Argument::getProgramName() const
{
    return programName_;
}

string Argument::getPathImage1() const
{
    return pathImage1_;
}

string Argument::getPathImage2() const
{
    return pathImage2_;
}

string Argument::getOutName() const
{
    return outName_;
}

string Argument::getPathBackgroundImage() const
{
    return pathBackgroundImage_;
}

void Argument::setPathProgram(const string& pathProgram)
{
    pathProgram_ = pathProgram;
}

void Argument::setProgramName(const string& programName)
{
    programName_ = programName;
}

void Argument::setPathImage1(const string& pathImage1)
{
    pathImage1_ = pathImage1;
}

void Argument::setPathImage2(const string& pathImage2)
{
    pathImage2_ = pathImage2;
}

void Argument::setOutName(const string& outName)
{
    outName_ = outName;
}

void Argument::setPathBackgroundImage(const string& pathBackgroundImage)
{
    pathBackgroundImage_ = pathBackgroundImage;
}