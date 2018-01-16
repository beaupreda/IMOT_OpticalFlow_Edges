#include "FlowProcessor.h"

using namespace cv;
using namespace std;

FlowProcessor::FlowProcessor()
{
}

bool FlowProcessor::areAngleOpposite(const Mat& angle1, const Mat& angle2)
{
    Point2i center1(angle1.cols / 2, angle1.rows / 2);
    Point2i center2(angle2.cols / 2, angle2.rows / 2);

    float centerAngle1 = angle1.at<float>(center1);
    float centerAngle2 = angle2.at<float>(center2);

    double difference = abs(centerAngle1- centerAngle2);

    return difference >= ANGLE_LOW && difference <= ANGLE_HIGH;
}

bool FlowProcessor::areMagnitudeSimilar(const double meanRegion1, const double standardDevRegion1,
                                        const double meanRegion2, const double standardDevRegion2)
{
    double minRegion1 = meanRegion1 - standardDevRegion1;
    double maxRegion1 = meanRegion1+ standardDevRegion1;

    double minRegion2 = meanRegion2 - standardDevRegion2;
    double maxRegion2 = meanRegion2 + standardDevRegion2;

    return ((minRegion1 >= minRegion2 && minRegion1 <= maxRegion2) ||
            (maxRegion1 >= minRegion2 && maxRegion1 <= maxRegion2) ||
            (minRegion2 >= minRegion1 && minRegion2 <= maxRegion1) ||
            (maxRegion2 >= minRegion1 && maxRegion2 <= maxRegion1));
}

bool FlowProcessor::isFlowSimilar(const Mat& magnitudeRegion1, const Mat& angleRegion1,
                                  const Mat& magnitudeRegion2, const Mat& angleRegion2)
{
    cv::Scalar meanMagnitude1, meanMagnitude2, standardDevMagnitude1, standardDevMagnitude2;

    cv::meanStdDev(magnitudeRegion1, meanMagnitude1, standardDevMagnitude1);
    cv::meanStdDev(magnitudeRegion2, meanMagnitude2, standardDevMagnitude2);

    return areMagnitudeSimilar(meanMagnitude1[0], standardDevMagnitude1[0],
                               meanMagnitude2[0], standardDevMagnitude2[0]) &&
           !areAngleOpposite(angleRegion1, angleRegion2);
}

vector<Rect> FlowProcessor::analyseClusters(const Mat &angle, const int index, const map<int, Rect>& boxes,
                                    map<int, bool>& flowModified)
{
    Rect unionBox;
    bool onlyOneBox = false;

    vector<Rect> flowBoxes;
    map<int, vector<double>> intersectArea = mapIntersectionRatio(boxes);

    for (auto iter = intersectArea.begin(); iter != intersectArea.end(); ++iter)
    {
        for (int i = 0; i < iter->second.size(); ++i)
        {
            if (iter->second[i] <= FLOW_THRESHOLD && checkBoxesDimensions(boxes.at(iter->first), boxes.at(i)) &&
                areAngleOpposite(angle(boxes.at(iter->first)), angle(boxes.at(i))))
            {
                flowBoxes.push_back(boxes.at(iter->first));
                flowBoxes.push_back(boxes.at(i));

                onlyOneBox = false;
                goto end;
            }
            else
            {
                unionBox = boxes.at(LITTLE) | boxes.at(MEDIUM) | boxes.at(BIG);
                onlyOneBox = true;
            }
        }
    }

    end:
    if (onlyOneBox)
    {
        flowBoxes.push_back(unionBox);
        flowModified.emplace(index, false);
    }
    else
    {
        flowModified.emplace(index, true);
    }

    return flowBoxes;
}

vector<Rect> FlowProcessor::process(const Mat& flowRegion, const Mat& angle, const Point2i& point,
                            map<int, bool>& flowModified, const int index)
{
    map<int, Rect> clusterBoxes;
    map<int, Point2i> clusterCenters;
    map<int, vector<Point2i>> clusterPoints;

    kmeans(flowRegion, clusterBoxes, clusterCenters, clusterPoints);

    vector<Rect> boxes = analyseClusters(angle, index, clusterBoxes, flowModified);

    for (int j = 0; j < boxes.size(); ++j)
        boxes[j] = Rect(boxes[j].x + point.x, boxes[j].y + point.y,
                        boxes[j].width, boxes[j].height);

    return boxes;
}

void FlowProcessor::kmeans(const Mat& flowRegion,
                           map<int, Rect>& clusterBoxes,
                           map<int, Point2i>& clusterCenters,
                           map<int, vector<Point2i>>& clusterPoints)
{
    Mat center, labels, flow;

    if (flowRegion.rows * flowRegion.cols < K)
    {
        Mat temp = Mat::zeros(flowRegion.rows + 1, flowRegion.cols + 1, flowRegion.type());
        temp.copyTo(flow);
    }
    else
    {
        flowRegion.copyTo(flow);
    }

    double compactness = cv::kmeans(flow.reshape(1, flow.rows * flow.cols), K, labels,
                                TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 100, 0.1),
                                15, KMEANS_PP_CENTERS, center);

    vector<Point2i> cluster;

    for (int i = 0; i < K; ++i)
    {
        cluster.clear();
        for (int j = 0; j < labels.rows; ++j)
        {
            if (i == labels.at<int>(j, 0))
            {
                Point2i index = Point2i(j % flow.cols, j / flow.cols);
                cluster.push_back(index);
            }
        }
        clusterPoints.emplace(i, cluster);
    }

    for (auto iter = clusterPoints.begin(); iter != clusterPoints.end(); ++iter)
    {
        Point2i topLeftPoint = findTopLeftPoint(iter->second);
        Point2i bottomRightPoint = findBottomRightPoint(iter->second);

        clusterBoxes.emplace(iter->first, Rect(topLeftPoint, bottomRightPoint));
    }

    for (auto iter = clusterPoints.begin(); iter != clusterPoints.end(); ++iter)
        clusterCenters.emplace(iter->first, findCenterPosition(iter->second));

}

map<BoxArea, int> FlowProcessor::mapAreaToBoxes(const map<int, vector<Point2i>>& points)
{
    vector<int> area;

    for (auto it = points.begin(); it != points.end(); ++it)
        area.push_back(static_cast<int>(it->second.size()));

    sort(area.begin(), area.end());

    map<BoxArea, int> mappedAreas;

    for (auto it = points.begin(); it != points.end(); ++it)
        for (int i = 0; i < area.size(); ++i)
            if (area[i] == it->second.size())
                mappedAreas.emplace(static_cast<BoxArea>(i), it->first);

    return mappedAreas;
}

map<int, vector<double>> FlowProcessor::mapIntersectionRatio(const map<int, Rect>& boxes)
{
    map<int, vector<double>> mapIoU;

    for (auto it_clusters = boxes.begin(); it_clusters != boxes.end(); ++it_clusters)
    {
        for (auto it = boxes.begin(); it != boxes.end(); ++it)
        {
            double intersectRatio = intersectionSmallestBox(it_clusters->second, it->second);
            mapIoU[it_clusters->first].push_back(intersectRatio);
        }
    }

    return mapIoU;
}

vector<Rect> FlowProcessor::mergeSimilarFlow(const Mat& magnitude, const Mat& angle,
                                             const vector<vector<Point>>& contours)
{
    vector<Rect> boxes;

    if (contours.size() == 1)
    {
        boxes.push_back(boundingRect(contours[0]));
        goto end;
    }

    for (int i = 0; i < contours.size(); ++i)
    {
        for (int j = i + 1; j < contours.size(); ++j)
        {
            int distance = computeDistanceBlobs(contours[i], contours[j]);

            Rect box1 = boundingRect(Mat(contours[i]));
            Rect box2 = boundingRect(Mat(contours[j]));

            int intersectArea = (box1 & box2).area();
            int angleBetweenBoxes = computeAngleBoxes(box1, box2);

            // angle in range [0, 360]
            if (angleBetweenBoxes < 0)
                angleBetweenBoxes += 360;

            bool validAngle = angleIsValid(angleBetweenBoxes);

            if ((distance <= MIN_DISTANCE || intersectArea > 0) && validAngle)
            {
                Mat magnitudeRegion1, magnitudeRegion2;

                magnitude(box1).copyTo(magnitudeRegion1);
                magnitude(box2).copyTo(magnitudeRegion2);

                Mat angleRegion1, angleRegion2;

                angle(box1).copyTo(angleRegion1);
                angle(box2).copyTo(angleRegion2);

                if (isFlowSimilar(magnitudeRegion1, angleRegion1, magnitudeRegion2, angleRegion2))
                {
                    Rect unionRect = box1 | box2;
                    boxes.push_back(unionRect);
                }
                else
                {
                    boxes.push_back(box1);
                    boxes.push_back(box2);
                }
            }
            else
            {
                boxes.push_back(box1);
                boxes.push_back(box2);
            }
        }
    }

    end:
    return boxes;
}

map<int, int> FlowProcessor::regroupCentersCluster(const map<int, vector<Point2i>>& points,
                                                   const map<int, Point2i>& centers)
{
    auto firstCenter = centers.begin();

    auto secondCenter= centers.begin();
    secondCenter++;

    auto thirdCenter = centers.begin();
    thirdCenter++;
    thirdCenter++;

    map<int, int> centersCluster;

    for (auto iter = points.begin(); iter != points.end(); ++iter)
    {
        for (auto it = iter->second.begin(); it != iter->second.end(); ++it)
        {
            if (firstCenter->second == *it)
                centersCluster[firstCenter->first] = iter->first;

            if (secondCenter->second == *it)
                centersCluster[secondCenter->first] = iter->first;

            if (thirdCenter->second == *it)
                centersCluster[thirdCenter->first] = iter->first;
        }
    }

    return centersCluster;
}
