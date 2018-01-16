#include "Line.h"

using namespace cv;
using namespace std;

Line::Line()
{
    startPoint_ = Point2i(0, 0);
    endPoint_ = Point2i(0, 0);
    direction_ = X;
}

Line::Line(cv::Point2i startPt, cv::Point2i endPt, ConstantDirection direction)
{
    startPoint_ = startPt;
    endPoint_ = endPt;
    direction_ = direction;
}

Point2i Line::getStartPoint() const
{
    return startPoint_;
}

Point2i Line::getEndPoint() const
{
    return endPoint_;
}

ConstantDirection Line::getConstantDirection() const
{
    return direction_;
}

void Line::setStartPoint(const cv::Point2i& startPt)
{
    startPoint_ = startPt;
}

void Line::setEndPoint(const cv::Point2i& endPt)
{
    endPoint_ = endPt;
}

void Line::setConstantDirection(const ConstantDirection& direction)
{
    direction_ = direction;
}

int Line::computeDistance(std::shared_ptr<Line> line, const ConstantDirection& direction)
{
    int distance = INT32_MAX;

    if (direction == X)
        distance = abs(this->startPoint_.x - line->startPoint_.x);

    else if (direction == Y)
        distance = abs(this->startPoint_.y - line->startPoint_.y);

    return distance;
}

shared_ptr<Line> Line::getSmallestCommonLine(std::shared_ptr<Line> line, const ConstantDirection& direction)
{
    shared_ptr<Line> commonLine (new Line());

    commonLine->setConstantDirection(direction);

    if (direction == X)
    {
        int largestStartPoint = max(this->startPoint_.y, line->startPoint_.y);
        int smallestEndPoint = min(this->endPoint_.y, line->endPoint_.y);
        int x = min(this->endPoint_.x, line->endPoint_.x);

        // makes sure that startPoint and endPoint were not reversed
        int largestPoint = min(largestStartPoint, smallestEndPoint);
        int smallestPoint = max(largestStartPoint, smallestEndPoint);

        commonLine->setStartPoint(Point2i(x, smallestPoint));
        commonLine->setEndPoint(Point2i(x, largestPoint));

        // verify that the commonLine has an intersection with both lines
        if ((commonLine->startPoint_.y == this->endPoint_.y && commonLine->endPoint_.y == line->startPoint_.y) ||
            (commonLine->startPoint_.y == line->endPoint_.y && commonLine->endPoint_.y == this->startPoint_.y))
        {
            commonLine->setStartPoint(Point2i(0, 0));
            commonLine->setEndPoint(Point2i(0, 0));
        }
    }

    if (direction == Y)
    {
        int largestStartPoint = max(this->startPoint_.x, line->startPoint_.x);
        int smallestEndPoint = min(this->endPoint_.x, line->endPoint_.x);
        int y = min(this->endPoint_.y, line->endPoint_.y);

        // makes sure that startPoint and endPoint were not reversed
        int largestPoint = min(largestStartPoint, smallestEndPoint);
        int smallestPoint = max(largestStartPoint, smallestEndPoint);

        commonLine->setStartPoint(Point2i(smallestPoint, y));
        commonLine->setEndPoint(Point2i(largestPoint, y));

        // verify that the commonLine has an intersection with both lines
        if ((commonLine->startPoint_.x == this->endPoint_.x && commonLine->endPoint_.x == line->startPoint_.x) ||
            (commonLine->startPoint_.x == line->endPoint_.x && commonLine->endPoint_.x == this->startPoint_.x))
        {
            commonLine->setStartPoint(Point2i(0, 0));
            commonLine->setEndPoint(Point2i(0, 0));
        }
    }

    return commonLine;
}

bool Line::hasIntersection(std::shared_ptr<Line> line, const ConstantDirection& direction, const cv::Mat& image)
{
    if (direction == X)
    {
        for (int y = 0; y < image.rows; y++)
            if (y >= this->startPoint_.y && y <= this->endPoint_.y && y >= line->startPoint_.y && y <= line->endPoint_.y)
                return true;
    }

    if (direction == Y)
    {
        for (int x = 0; x < image.cols; x++)
            if (x >= this->startPoint_.x && x <= this->endPoint_.x && x >= line->startPoint_.x && x <= line->endPoint_.x)
                return true;
    }
}
