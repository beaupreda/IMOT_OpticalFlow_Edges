#include "EdgeProcessor.h"

using namespace cv;
using namespace std;

EdgeProcessor::EdgeProcessor()
{
}

void EdgeProcessor::automaticCannyDetector(Mat& grayImage, double& low, double& high, bool isCaliber)
{
    if (isCaliber)
    {
        double medianValue = median(grayImage);

        low = max(static_cast<double>(0), (1 - SIGMA) * medianValue);
        high = min(static_cast<double>(255), (1 + SIGMA) * medianValue);
    }

    cv::Canny(grayImage, grayImage, low, high);
}

vector<Rect> EdgeProcessor::process(const Mat& region, const Mat& backgroundImage, const Point2i& point)
{
    Mat grayRegion, edges, edgesMap, backgroundGray, backgroundEdges, backgroundMap;

    cvtColor(region, grayRegion, CV_BGR2GRAY);
    cvtColor(backgroundImage, backgroundGray, CV_BGR2GRAY);

    assert(grayRegion.type() == CV_8UC1);

    blur(grayRegion, edges, cv::Size(3, 3));
    blur(backgroundGray, backgroundEdges, cv::Size(3, 3));

    double low = 0.0; double high = 0.0;
    automaticCannyDetector(edges, low, high, true);
    automaticCannyDetector(backgroundEdges, low, high, false);

    edgesMap = Scalar::all(0);
    region.copyTo(edgesMap, edges);

    backgroundMap = Scalar::all(0);
    backgroundImage.copyTo(backgroundMap, backgroundEdges);

    Mat edgesBinary(edgesMap.size(), CV_8UC1);
    edgesBinary = edgesToBinary(edgesMap);

    Mat backgroundBinary(backgroundEdges.size(), CV_8UC1);
    backgroundBinary = edgesToBinary(backgroundMap);

    Mat newBinaryEdges(edgesBinary.size(), CV_8UC1);
    bitwise_xor(edgesBinary, backgroundBinary, newBinaryEdges);

    vector<Point2i> edgePoints = extractPointsFromEdges(newBinaryEdges);
    map<int, vector<Point2i>> groupedPoints = groupEdges(edgePoints);

    vector<Rect> tmpBoxes = makeBoxesFromEdges(groupedPoints);

    vector<Rect> edgeBoxes = mergeBoxes(tmpBoxes);

    for (int i = 0; i < edgeBoxes.size(); ++i)
    {
        Rect rect;
        if (edgeBoxes[i] == Rect(0, 0, 0, 0))
            rect = Rect(point.x, point.y, region.size().width, region.size().height);
        else
            // adjust global offset of the image to the box
            rect = Rect(point.x + edgeBoxes[i].x, point.y + edgeBoxes[i].y,
                        edgeBoxes[i].width, edgeBoxes[i].height);

        edgeBoxes[i] = rect;
    }

    return edgeBoxes;
}

Mat EdgeProcessor::edgesToBinary(const Mat& edgeMap)
{
    Mat binary(edgeMap.size(), CV_8UC1);

    for (int y = 0; y < edgeMap.rows; ++y)
    {
        for (int x = 0; x < edgeMap.cols; ++x)
        {
            if (edgeMap.at<Vec3b>(y, x) == Vec3b(0, 0, 0))
                binary.at<uchar>(y, x) = 0;
            else
                binary.at<uchar>(y, x) = 255;
        }
    }

    return binary;
}

vector<Point2i> EdgeProcessor::extractPointsFromEdges(const cv::Mat& edgeMap)
{
    vector<Point2i> points;

    for (int y = 0; y < edgeMap.rows; ++y)
    {
        for (int x = 0; x < edgeMap.cols; ++x)
        {
            if (edgeMap.at<uchar>(y, x) == 255)
                points.push_back(cv::Point2i(x, y));
        }
    }

    return points;
}

map<int, bool> EdgeProcessor::initializeVisitedPoints(const vector<Point2i>& edgePoints)
{
    map<int, bool> visited;

    for (int i = 0; i < edgePoints.size(); ++i)
        visited.emplace(i, false);

    return visited;
}

int EdgeProcessor::getNextPoint(const map<int, bool>& pointsVisited)
{
    for (auto it = pointsVisited.begin(); it != pointsVisited.end(); ++it)
        if (!it->second)
            return it->first;

    return -1;
}

map<int, vector<Point2i>> EdgeProcessor::groupEdges(const vector<Point2i>& edgePoints)
{
    int groupId = 0;

    vector<Point2i> groupedPoints;
    queue<int> nextPoints;
    map<int, vector<cv::Point2i>> groupedEdges;

    if (edgePoints.size() == 0)
        return groupedEdges;
    else if (edgePoints.size() >= MAX_EDGES)
    {
        groupedEdges.emplace(groupId, edgePoints);
        return  groupedEdges;
    }

    map<int, bool> pointsVisited = initializeVisitedPoints(edgePoints);
    map<int, bool> pointsInQueue = initializeVisitedPoints(edgePoints);

    nextPoints.push(0);

    while (!arePointsAllVisited(pointsVisited))
    {
        int currentPointId = nextPoints.front();

        if (currentPointId == -1)
            return groupedEdges;

        Point2i currentPoint = edgePoints[currentPointId];
        pointsVisited[currentPointId] = true;
        nextPoints.pop();
        groupedPoints.push_back(currentPoint);

        for (int i = 0; i < edgePoints.size(); ++i)
        {
            if (!pointsVisited[i])
            {
                int dx = abs(currentPoint.x - edgePoints[i].x);
                int dy = abs(currentPoint.y - edgePoints[i].y);

                if (dx <= EDGE_OFFSET && dy <= EDGE_OFFSET && !pointsInQueue[i])
                {
                    nextPoints.push(i);
                    pointsInQueue[i] = true;
                }
            }
        }

        if (nextPoints.empty())
        {
            groupedEdges.emplace(groupId, groupedPoints);
            groupedPoints.clear();

            groupId++;

            nextPoints.push(getNextPoint(pointsVisited));
        }
    }

    return groupedEdges;
}

vector<Rect> EdgeProcessor::makeBoxesFromEdges(const map<int, vector<Point2i>>& groupedPoints)
{
    vector<Rect> allBoxes;
    vector<Rect> goodBoxes;

    for (auto iter = groupedPoints.begin(); iter != groupedPoints.end(); ++iter)
    {
        Point2i top_left = findTopLeftPoint(iter->second);
        Point2i bottom_right = findBottomRightPoint(iter->second);

        Rect box(top_left, bottom_right);
        allBoxes.push_back(box);
    }

    goodBoxes = eliminateSmallBoxes(allBoxes);
    goodBoxes = eliminateSmallBoxes(goodBoxes);

    return goodBoxes;
}

vector<Rect> EdgeProcessor::mergeBoxes(const vector<Rect>& boxes)
{
    vector<Rect> mergedBoxes;

    if (boxes.size() == 1)
        mergedBoxes.push_back(boxes[0]);
    else if (boxes.size() == 2)
    {
        double ratio = intersectionOverUnion(boxes[0], boxes[1]);

        if (ratio >= 0.15)
        {
            Rect union_rect = boxes[0] | boxes[1];
            mergedBoxes.push_back(union_rect);
        }
        else
        {
            mergedBoxes.push_back(boxes[0]);
            mergedBoxes.push_back(boxes[1]);
        }
    }
    else
    {
        for (int i = 0; i < boxes.size(); ++i)
            mergedBoxes.push_back(boxes[i]);
    }

    return mergedBoxes;
}

void EdgeProcessor::mergeSimilarBoxes(vector<Rect>& boxes, const Mat& source)
{
    for (int i = 0; i < boxes.size(); ++i)
    {
        for (int j = i + 1; j < boxes.size(); ++j)
        {
            int distance = computeDistanceBoxes(boxes[i], boxes[j], source);

            if (distance <= DISTANCE_THRESHOLD)
            {
                shared_ptr<Line> line1 (new Line());
                shared_ptr<Line> line2 (new Line());

                line1->setStartPoint(Point2i(boxes[i].x, boxes[i].y));
                line1->setEndPoint(Point2i(boxes[i].x + boxes[i].width, boxes[i].y));
                line1->setConstantDirection(Y);

                line2->setStartPoint(Point2i(boxes[j].x, boxes[j].y));
                line2->setEndPoint(Point2i(boxes[j].x + boxes[j].width, boxes[j].y));
                line1->setConstantDirection(Y);

                shared_ptr<Line> intersectLine;
                intersectLine = line1->getSmallestCommonLine(line2, Y);

                int width = intersectLine->getEndPoint().x - intersectLine->getStartPoint().x;

                double widthRatio1 = (double) width / (double) boxes[i].width;
                double widthRatio2 = (double) width / (double) boxes[j].width;

                if (widthRatio1 >= SIMILARITY_RATIO || widthRatio2>= SIMILARITY_RATIO)
                {
                    Rect union_rect = boxes[i] | boxes[j];

                    boxes[i] = cv::Rect(0, 0, 0, 0);
                    boxes[j] = cv::Rect(0, 0, 0, 0);
                    boxes.push_back(union_rect);

                    eliminateZeroBoxes(boxes);

                    mergeSimilarBoxes(boxes, source);
                }
            }
        }
    }
}

bool EdgeProcessor::arePointsAllVisited(const map<int, bool>& pointsVisited)
{
    for (auto it = pointsVisited.begin(); it != pointsVisited.end(); ++it)
        if (!it->second)
            return false;

    return true;
}

double EdgeProcessor::median(const Mat& channel)
{
    double m = (channel.rows * channel.cols) / 2;
    int bin = 0;
    double med = -1.0;

    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    bool uniform = true;
    bool accumulate = false;

    Mat hist;
    calcHist(&channel, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

    for (int i = 0; i < histSize && med < 0.0; ++i)
    {
        bin += cvRound(hist.at<float>(i));
        if (bin > m && med < 0.0)
            med = i;
    }

    return med;
}
