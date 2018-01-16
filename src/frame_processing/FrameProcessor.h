#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"
#include "../support/Line.h"
#include "../support/Place.h"
#include "../support/FileOperations.h"

#ifndef IMOT_OPTICALFLOW_EDGES_FRAMEPROCESSOR_H
#define IMOT_OPTICALFLOW_EDGES_FRAMEPROCESSOR_H

const double ALPHA = 0.01;
const double CUTOFF = 0.25;
const double SIMILARITY = 0.85;
const double THRESHOLD = 0.15;

const int PIXEL_OFFSET = 8;
const int Q1_LOW = 39;
const int Q1_HIGH = 51;
const int Q2_LOW = 129;
const int Q2_HIGH = 141;
const int Q3_LOW = 219;
const int Q3_HIGH = 231;
const int Q4_LOW = 309;
const int Q4_HIGH = 321;

class FrameProcessor
{
public:

    FrameProcessor();

    bool checkBoxesDimensions(const cv::Rect& box1, const cv::Rect& box2) const;
    bool angleIsValid(const int angle);

    int computeAngleBoxes(const cv::Rect& box1, const cv::Rect& box2);
    int computeDistanceBoxes(const cv::Rect& box1, const cv::Rect& box2, const cv::Mat& image);
    int computeDistanceBlobs(const std::vector<cv::Point2i>& region1, const std::vector<cv::Point2i>& region2);

    cv::Mat createBackgroundImage(std::shared_ptr<Place> place, std::shared_ptr<FileOperations> fileOperator);

    std::vector<cv::Rect> eliminateBoxesIncluded(const std::vector<cv::Rect>& boxes);
    std::vector<cv::Rect> eliminateSmallBoxes(const std::vector<cv::Rect>& boxes);
    std::vector<cv::Rect> eliminateCloseRegions(const std::vector<cv::Rect>& boxes);
    void eliminateZeroBoxes(std::vector<cv::Rect>& boxes);

    std::vector<std::vector<cv::Point>> extractContours(const cv::Mat& binaryImage);
    std::vector<cv::Rect> extractInterestRegions(const cv::Mat& binaryImage);
    std::vector<cv::Rect> extractBoxes(std::vector<cv::Rect> boxes);

    cv::Point2i findTopLeftPoint(const std::vector<cv::Point2i>& points);
    cv::Point2i findBottomRightPoint(const std::vector<cv::Point2i>& points);
    cv::Point2i findCenterPosition(const std::vector<cv::Point2i>& points);
    cv::Rect findSmallestBox(const cv::Rect& box1, const cv::Rect& box2);

    bool boxesAreSimilar(const cv::Rect& rect1, const cv::Rect& rect2);
    bool boxIsIncluded(const cv::Rect& rect1, const cv::Rect& rect2);

    double intersectionOverUnion(const cv::Rect& box1, const cv::Rect& box2);
    double intersectionSmallestBox(const cv::Rect& box1, const cv::Rect& box2);

    static void displayImage(const cv::Mat& image, const std::string& name)
    {
        cv::namedWindow(name, CV_WINDOW_NORMAL);
        imshow(name, image);

        cv::waitKey(0);
    }

};

#endif //IMOT_OPTICALFLOW_EDGES_FRAMEPROCESSOR_H
