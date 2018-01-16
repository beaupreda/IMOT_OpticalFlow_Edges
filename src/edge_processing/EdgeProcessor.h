#include "../frame_processing/FrameProcessor.h"

#ifndef IMOT_OPTICALFLOW_EDGES_EDGEPROCESSOR_H
#define IMOT_OPTICALFLOW_EDGES_EDGEPROCESSOR_H

const double SIGMA = 0.33;
const double SIMILARITY_RATIO = 0.50;

const int DISTANCE_THRESHOLD = 3;
const int EDGE_OFFSET = 3;
const int MAX_EDGES = 1200;

class EdgeProcessor: public FrameProcessor
{
public:

    EdgeProcessor();

    void automaticCannyDetector(cv::Mat& grayImage, double& low, double& high, bool isCaliber);

    std::vector<cv::Rect> process(const cv::Mat& region, const cv::Mat& backgroundImage, const cv::Point2i& point);

    cv::Mat edgesToBinary(const cv::Mat& edgeMap);

    std::vector<cv::Point2i> extractPointsFromEdges(const cv::Mat& edgeMap);

    std::map<int, bool> initializeVisitedPoints(const std::vector<cv::Point2i>& edgePoints);
    int getNextPoint(const std::map<int, bool>& pointsVisited);

    std::map<int, std::vector<cv::Point2i>> groupEdges(const std::vector<cv::Point2i>& edgePoints);

    std::vector<cv::Rect> makeBoxesFromEdges(const std::map<int, std::vector<cv::Point2i>>& groupedPoints);

    std::vector<cv::Rect> mergeBoxes(const std::vector<cv::Rect>& boxes);
    void mergeSimilarBoxes(std::vector<cv::Rect>& boxes, const cv::Mat& source);

    bool arePointsAllVisited(const std::map<int, bool>& pointsVisited);

    double median(const cv::Mat& channel);

};

#endif //IMOT_OPTICALFLOW_EDGES_EDGEPROCESSOR_H
