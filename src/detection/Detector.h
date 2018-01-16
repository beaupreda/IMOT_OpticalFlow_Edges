#include "opencv2/optflow.hpp"
#include "../edge_processing/EdgeProcessor.h"
#include "../flow_processing/FlowProcessor.h"

#ifndef IMOT_OPTICALFLOW_EDGES_DETECTOR_H
#define IMOT_OPTICALFLOW_EDGES_DETECTOR_H

const double THRESHOLD_COMP = 0.65;
const double THRESHOLD_CHANGE = 0.30;

const int PADDING = 1;

class Detector
{
public:

    Detector();

    std::shared_ptr<EdgeProcessor> getEdgeProcessor() const;
    std::shared_ptr<FlowProcessor> getFlowProcessor() const;

    std::vector<cv::Rect> compareEdgesFlow(const std::vector<cv::Rect>& edges,
                                           const std::vector<cv::Rect>& flow,
                                           const std::map<int, bool>& mergedModified,
                                           const std::map<int, bool>& edgesModified,
                                           const std::map<int, bool>& flowModified,
                                           const int index);

    std::map<int, bool> makeModifiedEdgesMap(const std::vector<cv::Rect>& regions,
                                             const std::map<int, std::vector<cv::Rect>>& boxes);
    std::map<int, bool> makeModifiedFlowMap(const cv::Mat& angle,
                                            const std::vector<cv::Rect>& regions,
                                            const std::map<int, bool>& flowModified,
                                            const std::map<int, std::vector<cv::Rect>>& boxes);

    cv::Mat processFrame(std::shared_ptr<Place> place, const int frameNumber,
                         const std::string& flowName,
                         const std::string& sourceName,
                         const std::string& backgroundName);

    cv::Mat createNewBackgroundImage(std::shared_ptr<Place> place, cv::Size size, const int frameNumber,
                                     const std::vector<cv::Rect>& boxes, const cv::Mat& source);

    bool areAdjacentFaces(const cv::Rect& box1, const cv::Rect& box2, cv::Rect& intersectionRect,
                          const cv::Mat& source);
    bool areCornersClose(const cv::Rect& box1, const cv::Rect& box2);
    bool isBoxTaken(const std::vector<cv::Rect>& boxesTaken, const cv::Rect& proposedBox);

    std::map<int, bool> areEquivalentRegions(const cv::Mat& source,
                                             const std::vector<cv::Rect>& before,
                                             const std::vector<cv::Rect>& after);

private:

    std::shared_ptr<EdgeProcessor> edgeProcessor_;
    std::shared_ptr<FlowProcessor> flowProcessor_;

};

#endif //IMOT_OPTICALFLOW_EDGES_DETECTOR_H
