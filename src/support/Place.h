#include <memory>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"

#ifndef IMOT_OPTICALFLOW_EDGES_PLACE_H
#define IMOT_OPTICALFLOW_EDGES_PLACE_H

#define MIN_RENE_FRAME 7200
#define MAX_RENE_FRAME 8199

#define MIN_ROUEN_FRAME 20
#define MAX_ROUEN_FRAME 620

#define MIN_SHERBROOKE_FRAME 2754
#define MAX_SHERBROOKE_FRAME 3755

#define MIN_STMARC_FRAME 1000
#define MAX_STMARC_FRAME 1999

#define BG_OFFSET_ONE -1
#define BG_OFFSET_ZERO 0

class Place
{
public:

    Place();
    Place(std::string folder, std::string subFolder, std::string backgroundFolder,
          std::string maskName, int backgroundOffset);

    std::string getFolder() const;
    std::string getSubFolder() const;
    std::string getBackgroundFolder() const;
    std::string getMaskName() const;
    int getBackgroundOffset() const;
    cv::Mat getMask() const;
    cv::Mat getBackgroundImage() const;
    std::string getSpecificBackground(int index) const;

    void addFrames(const std::vector<std::string>& frames);
    void addBackgroundFrames(const std::vector<std::string>& backgroundFrames);
    void addBackgroundImage(const cv::Mat& backgroundImage);
    void setMask(const cv::Mat& mask);

private:

    std::string folder_;
    std::string subFolder_;
    std::string backgroundFolder_;
    std::string maskName_;

    int backgroundOffset_;

    std::vector<std::string> frames_;
    std::vector<std::string> backgroundFrames_;

    cv::Mat backgroundImage_;
    cv::Mat mask_;

};

#endif //IMOT_OPTICALFLOW_EDGES_PLACE_H
