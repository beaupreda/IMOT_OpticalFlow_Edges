#include <string>

#ifndef IMOT_OPTICALFLOW_EDGES_ARGUMENT_H
#define IMOT_OPTICALFLOW_EDGES_ARGUMENT_H

class Argument
{
public:

    Argument();
    Argument(std::string pathProgram, std::string programName, std::string pathImage1,
              std::string pathImage2, std::string outName, std::string pathBackgroundImage);

    std::string getPathProgram() const;
    std::string getProgramName() const;
    std::string getPathImage1() const;
    std::string getPathImage2() const;
    std::string getOutName() const;
    std::string getPathBackgroundImage() const;

    void setPathProgram(const std::string& pathProgram);
    void setProgramName(const std::string& programName);
    void setPathImage1(const std::string& pathImage1);
    void setPathImage2(const std::string& pathImage2);
    void setOutName(const std::string& outName);
    void setPathBackgroundImage(const std::string& pathBackgroundImage);

private:

    std::string pathProgram_;
    std::string programName_;
    std::string pathImage1_;
    std::string pathImage2_;
    std::string outName_;
    std::string pathBackgroundImage_;

};

#endif //IMOT_OPTICALFLOW_EDGES_ARGUMENT_H
