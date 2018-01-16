#include "../frame_processing/FrameProcessor.h"

#ifndef IMOT_OPTICALFLOW_EDGES_FLOWPROCESSOR_H
#define IMOT_OPTICALFLOW_EDGES_FLOWPROCESSOR_H

const double FLOW_THRESHOLD = 0.40;

const int ANGLE_LOW = 90;
const int ANGLE_HIGH = 270;
const int MIN_DISTANCE = 7;
const int K = 3;

enum BoxArea {LITTLE = 0, MEDIUM = 1, BIG = 2};

class FlowProcessor: public FrameProcessor
{
public:

    FlowProcessor();

    bool areAngleOpposite(const cv::Mat& angle1, const cv::Mat& angle2);
    bool areMagnitudeSimilar(const double meanRegion1, const double standardDevRegion1,
                             const double meanRegion2, const double standardDevRegion2);
    bool isFlowSimilar(const cv::Mat& magnitudeRegion1, const cv::Mat& angleRegion1,
                       const cv::Mat& magnitudeRegion2, const cv::Mat& angleRegion2);

    std::vector<cv::Rect> analyseClusters(const cv::Mat& angle, const int index,
                                          const std::map<int, cv::Rect>& boxes,
                                          std::map<int, bool>& flowModified);
    std::vector<cv::Rect> process(const cv::Mat& flowRegion, const cv::Mat& angle, const cv::Point2i& point,
                                  std::map<int, bool>& flowModified, const int index);

    void kmeans(const cv::Mat& flowRegion,
                std::map<int, cv::Rect>& clusterBoxes,
                std::map<int, cv::Point2i>& clusterCenters,
                std::map<int, std::vector<cv::Point2i>>& clusterPoints);

    std::map<BoxArea, int> mapAreaToBoxes(const std::map<int, std::vector<cv::Point2i>>& points);
    std::map<int, std::vector<double>> mapIntersectionRatio(const std::map<int, cv::Rect>& boxes);

    std::vector<cv::Rect> mergeSimilarFlow(const cv::Mat& magnitude, const cv::Mat& angle,
                                           const std::vector<std::vector<cv::Point>>& contours);

    std::map<int, int> regroupCentersCluster(const std::map<int, std::vector<cv::Point2i>>& points,
                                             const std::map<int, cv::Point2i>& centers);

};

#endif //IMOT_OPTICALFLOW_EDGES_FLOWPROCESSOR_H
