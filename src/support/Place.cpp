#include "Place.h"

using namespace cv;
using namespace std;

Place::Place()
{
}

Place::Place(std::string subFolder, std::string backgroundFolder, std::string maskName)
{
    subFolder_ = subFolder;
    backgroundFolder_ = backgroundFolder;
    maskName_ = maskName;
}

Place::Place(string folder, string subFolder, string backgroundFolder,
             string maskName, int backgroundOffset)
{
    folder_ = folder;
    subFolder_ = subFolder;
    backgroundFolder_ = backgroundFolder;
    backgroundOffset_ = backgroundOffset;
    maskName_ = maskName;
}

string Place::getFolder() const
{
    return folder_;
}

string Place::getSubFolder() const
{
    return subFolder_;
}

string Place::getBackgroundFolder() const
{
    return backgroundFolder_;
}

string Place::getMaskName() const
{
    return maskName_;
}

int Place::getBackgroundOffset() const
{
    return backgroundOffset_;
}

Mat Place::getMask() const
{
    return mask_;
}

Mat Place::getBackgroundImage() const
{
    return backgroundImage_;
}

string Place::getSpecificBackground(int index) const
{
    if (index < backgroundFrames_.size())
        return backgroundFrames_[index];

    return NULL;
}

void Place::addFrames(const std::vector<std::string>& frames)
{
    frames_ = frames;
}

void Place::addBackgroundFrames(const std::vector<std::string>& backgroundFrames)
{
    backgroundFrames_ = backgroundFrames;
}

void Place::addBackgroundImage(const Mat& backgroundImage)
{
    backgroundImage_ = backgroundImage;
}

void Place::setMask(const cv::Mat& mask)
{
    mask_ = mask;
}

void Place::setFolder(const std::string& folder)
{
    folder_ = folder;
}

void Place::setBackgroundOffset(const int backgroundOffset)
{
    backgroundOffset_ = backgroundOffset;
}
