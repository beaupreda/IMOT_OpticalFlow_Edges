#include <memory>

#include "opencv2/opencv.hpp"

#ifndef IMOT_OPTICALFLOW_EDGES_LINE_H
#define IMOT_OPTICALFLOW_EDGES_LINE_H

enum ConstantDirection {X = 0, Y = 1};

class Line
{
public:

    Line();
    Line(cv::Point2i startPt, cv::Point2i endPt, ConstantDirection direction);

    cv::Point2i getStartPoint() const;
    cv::Point2i getEndPoint() const;
    ConstantDirection getConstantDirection() const;

    void setStartPoint(const cv::Point2i& startPt);
    void setEndPoint(const cv::Point2i& endPt);
    void setConstantDirection(const ConstantDirection& direction);

    int computeDistance(std::shared_ptr<Line> line, const ConstantDirection& direction);

    std::shared_ptr<Line> getSmallestCommonLine(std::shared_ptr<Line> line, const ConstantDirection& direction);

    bool hasIntersection(std::shared_ptr<Line> line, const ConstantDirection& direction, const cv::Mat& image);

private:

    cv::Point2i startPoint_;
    cv::Point2i endPoint_;
    ConstantDirection direction_;

};

#endif //IMOT_OPTICALFLOW_EDGES_LINE_H
