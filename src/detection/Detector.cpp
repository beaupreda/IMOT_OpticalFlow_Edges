#include "Detector.h"

using namespace cv;
using namespace std;

Detector::Detector(): edgeProcessor_ (new EdgeProcessor()), flowProcessor_ (new FlowProcessor())
{
}

shared_ptr<EdgeProcessor> Detector::getEdgeProcessor() const
{
    return edgeProcessor_;
}

shared_ptr<FlowProcessor> Detector::getFlowProcessor() const
{
    return flowProcessor_;
}

vector<Rect> Detector::compareEdgesFlow(const vector<Rect>& edges, const vector<Rect>& flow,
                                        const map<int, bool>& mergedModified,
                                        const map<int, bool>& edgesModified,
                                        const map<int, bool>& flowModified, const int index)
{
    vector<Rect> mergeBoxes;

    if (flowModified.at(index))
    {
        mergeBoxes.push_back(flow[0]);
        mergeBoxes.push_back(flow[1]);
    }
    else if (mergedModified.at(index) && edgesModified.at(index))
    {
        for (int i = 0; i < edges.size(); ++i)
            mergeBoxes.push_back(edges[i]);
    }
    else if (edges.size() >= 4)
    {
        mergeBoxes.push_back(flow[0]);
    }
    else if (edges.size() == 2 && flow.size() == 1)
    {
        int unionArea = (edges[0] | edges[1]).area();
        int flowArea = flow[0].area();

        double ratio = static_cast<double>(unionArea) / static_cast<double>(flowArea);

        if (ratio <= THRESHOLD_COMP)
            mergeBoxes.push_back(flow[0]);
        else
        {
            mergeBoxes.push_back(edges[0]);
            mergeBoxes.push_back(edges[1]);
        }
    }
    else
    {
        for (int i = 0; i < edges.size(); ++i)
            mergeBoxes.push_back(edges[i]);
    }

    return mergeBoxes;
}

map<int, bool> Detector::makeModifiedEdgesMap(const vector<Rect>& regions,
                                              const map<int, vector<Rect>>& boxes)
{
    map<int, bool> mapModified;

    auto iter = boxes.begin();
    int index = 0;

    while (index < regions.size())
    {
        if (iter->second.size() == 1)
        {
            int original_area = regions[index].area();
            int new_area = iter->second[0].area();

            double ratio_area = static_cast<double>(new_area) / static_cast<double>(original_area);

            if (ratio_area <= THRESHOLD_CHANGE)
                mapModified.emplace(iter->first, true);
            else
                mapModified.emplace(iter->first, false);
        }
        else
        {
            mapModified.emplace(iter->first, true);
        }
        ++index;
        ++iter;
    }

    return mapModified;
}

map<int, bool> Detector::makeModifiedFlowMap(const Mat& angle,
                                             const vector<Rect>& regions,
                                             const map<int, bool>& flowModified,
                                             const map<int, vector<Rect>>& boxes)
{
    map<int, bool> mappedFlow;

    auto it = boxes.begin();

    for (auto iter = flowModified.begin(); iter != flowModified.end(); ++iter)
    {
        if (iter->second)
        {
            Point2i center1(it->second[0].x + it->second[0].width / 2, it->second[0].y + it->second[0].height / 2);
            Point2i center2(it->second[1].x + it->second[1].width / 2, it->second[1].y + it->second[1].height / 2);

            double angle1 = angle.at<float>(center1);
            double angle2 = angle.at<float>(center2);

            double diff = abs(angle1 - angle2);

            if (diff >= ANGLE_LOW && diff <= ANGLE_HIGH)
                mappedFlow.emplace(iter->first, true);
            else
                mappedFlow.emplace(iter->first, false);
        }
        else
        {
            int original_area = regions[iter->first].area();
            int new_area = it->second[0].area();

            double ratio_area = static_cast<double>(new_area) / static_cast<double>(original_area);

            if (ratio_area <= THRESHOLD_CHANGE)
                mappedFlow.emplace(iter->first, true);
            else
                mappedFlow.emplace(iter->first, false);
        }
        ++it;
    }

    return mappedFlow;
}

Mat Detector::processFrame(shared_ptr<Place> place, const int frameNumber,
                           const string& flowName,
                           const string& sourceName,
                           const string& backgroundName)
{
    Mat flow = optflow::readOpticalFlow(flowName);
    Mat source = imread(sourceName, CV_LOAD_IMAGE_COLOR);
    Mat background = imread(backgroundName, CV_LOAD_IMAGE_UNCHANGED);

    Mat mask, res;

    res = edgeProcessor_->edgesToBinary(place->getMask());

    background = background & res;

    Mat backgroundImage = place->getBackgroundImage();

    Mat channels[2], magnitude, angle;
    split(flow, channels);
    cartToPolar(channels[0], channels[1], magnitude, angle, true);

    vector<vector<Point>> contours = edgeProcessor_->extractContours(background);
    vector<Rect> regions = edgeProcessor_->extractInterestRegions(background);
    vector<Rect> backgroundRegions = edgeProcessor_->eliminateBoxesIncluded(regions);

    vector<Rect> mergeRegions = flowProcessor_->mergeSimilarFlow(magnitude, angle, contours);

    vector<Rect> distinctRegions = edgeProcessor_->eliminateBoxesIncluded(mergeRegions);
    distinctRegions = edgeProcessor_->eliminateCloseRegions(distinctRegions);

    map<int, bool> mergeModified = areEquivalentRegions(source, backgroundRegions, distinctRegions);

    map<int, bool> flowModified;
    map<int, vector<Rect>> newEdgeBoxes, newFlowBoxes;

    for (int i = 0; i < distinctRegions.size(); ++i)
    {

        Mat sourceRegion, flowRegion, backgroundRegion, magnitudeRegion, angleRegion;

        source(distinctRegions[i]).copyTo(sourceRegion);
        backgroundImage(distinctRegions[i]).copyTo(backgroundRegion);
        flow(distinctRegions[i]).copyTo(flowRegion);
        magnitude(distinctRegions[i]).copyTo(magnitudeRegion);
        angle(distinctRegions[i]).copyTo(angleRegion);

        Point2i point(distinctRegions[i].x, distinctRegions[i].y);

        vector<Rect> edgeBoxes = edgeProcessor_->process(sourceRegion, backgroundRegion, point);
        newEdgeBoxes.emplace(i, edgeBoxes);

        vector<Rect> flowBoxes = flowProcessor_->process(flowRegion, angleRegion, point, flowModified, i);
        newFlowBoxes.emplace(i, flowBoxes);
    }

    map<int, bool> edgesModified = makeModifiedEdgesMap(distinctRegions, newEdgeBoxes);

    vector<Rect> allBoxes;

    int counter = 0;

    auto iterEdges = newEdgeBoxes.begin();
    auto iterFlow = newFlowBoxes.begin();

    while (counter < distinctRegions.size())
    {
        if (frameNumber== 82)
            int b = 2;

        vector<Rect> mergedBoxes;
        mergedBoxes = compareEdgesFlow(iterEdges->second, iterFlow->second, mergeModified,
                                          edgesModified, flowModified, counter);

        for (int i = 0; i < mergedBoxes.size(); ++i)
            allBoxes.push_back(mergedBoxes[i]);

        counter++;
        iterEdges++;
        iterFlow++;
    }

    Mat newBackgroundImage = createNewBackgroundImage(place, source.size(), frameNumber, allBoxes, source);

    return newBackgroundImage;
}

Mat Detector::createNewBackgroundImage(shared_ptr<Place> place, Size size, const int frameNumber,
                                       const vector<Rect>& boxes, const Mat& source)
{
    Mat img(size, CV_8UC1);
    img = Scalar::all(0);

    vector<Rect> intersectionRectangles;

    for (int i = 0; i < boxes.size(); ++i)
    {
        for (int j = i + 1; j < boxes.size(); ++j)
        {
            Rect intersectRectangle = boxes[i] & boxes[j];
            Rect interRect;

            if (intersectRectangle.area() > 0)
            {
                if (intersectRectangle.y > 0)
                    intersectRectangle.y -= PADDING;

                if (intersectRectangle.x > 0)
                    intersectRectangle.x -= PADDING;

                if (intersectRectangle.width + intersectRectangle.x < img.cols - 2 * PADDING)
                    intersectRectangle.width += 2 * PADDING;

                if (intersectRectangle.height + intersectRectangle.y < img.rows - 2 * PADDING)
                    intersectRectangle.height += 2 * PADDING;

                Mat rect_int(intersectRectangle.size(), CV_8UC1);
                intersectionRectangles.push_back(intersectRectangle);

            }
            else if (areCornersClose(boxes[i], boxes[j]))
            {
                Rect miniRect1(boxes[i].tl(), boxes[j].br());
                Rect miniRect2(boxes[j].tl(), boxes[i].br());

                if (miniRect1.width == 0)
                    miniRect1.width = 1;

                if (miniRect1.height == 0)
                    miniRect1.height = 1;

                if (miniRect2.width == 0)
                    miniRect2.width = 1;

                if (miniRect2.height == 0)
                    miniRect2.height = 1;

                int minArea = min(miniRect1.area(), miniRect2.area());

                if (minArea == miniRect1.area())
                {
                    miniRect1.x -= PADDING;
                    miniRect1.y -= PADDING;
                    miniRect1.width += 2 * PADDING;
                    miniRect1.height += 2 * PADDING;

                    intersectionRectangles.push_back(miniRect1);
                }
                else
                {
                    miniRect2.x -= PADDING;
                    miniRect2.y -= PADDING;
                    miniRect2.width += 2 * PADDING;
                    miniRect2.height += 2 * PADDING;

                    intersectionRectangles.push_back(miniRect2);
                }
            }
            else if (areAdjacentFaces(boxes[i], boxes[j], interRect, source))
            {
                intersectionRectangles.push_back(interRect);
            }
        }

        Rect box = boxes[i];

        if (box.width == 0)
            box.width = PADDING;

        if (box.height == 0)
            box.height = PADDING;

        Mat rect(box.size(), CV_8UC1);
        rect = Scalar::all(255);

        add(img(box), rect, img(box));
    }

    for (int i = 0; i < intersectionRectangles.size(); ++i)
    {
        if (intersectionRectangles[i].x + intersectionRectangles[i].width > source.cols)
            intersectionRectangles[i].width = source.cols - intersectionRectangles[i].x;

        if (intersectionRectangles[i].y + intersectionRectangles[i].height > source.rows)
            intersectionRectangles[i].height = source.rows - intersectionRectangles[i].y;

        Mat interRect(intersectionRectangles[i].size(), CV_8UC1);
        interRect = Scalar::all(255);

        subtract(img(intersectionRectangles[i]), interRect, img(intersectionRectangles[i]));
    }

    return img;
}

bool Detector::areAdjacentFaces(const Rect& box1, const Rect& box2, Rect& intersectionRect, const Mat& source)
{
    int distance = edgeProcessor_->computeDistanceBoxes(box1, box2, source);

    if (distance <= PADDING)
    {
        shared_ptr<Line> line1 (new Line());
        shared_ptr<Line> line2 (new Line());

        line1->setStartPoint(Point2i(box1.x, box1.y));
        line1->setEndPoint(Point2i(box1.x + box1.width, box1.y));
        line1->setConstantDirection(Y);

        line2->setStartPoint(Point2i(box2.x, box2.y));
        line2->setEndPoint(Point2i(box2.x + box2.width, box2.y));
        line2->setConstantDirection(Y);

        shared_ptr<Line> intersectLine;
        intersectLine = line1->getSmallestCommonLine(line2, Y);

        if (intersectLine->getStartPoint() == Point2i(0, 0) && intersectLine->getEndPoint() == Point2i(0, 0))
        {
            line1->setStartPoint(Point2i(box1.x, box1.y));
            line1->setEndPoint(Point2i(box1.x, box1.y + box1.height));
            line1->setConstantDirection(X);

            line2->setStartPoint(Point2i(box2.x, box2.y));
            line2->setEndPoint(Point2i(box2.x, box2.y + box2.height));
            line2->setConstantDirection(X);

            intersectLine = line1->getSmallestCommonLine(line2, X);

            if (intersectLine->getStartPoint() == Point2i(0, 0) && intersectLine->getEndPoint() == Point2i(0, 0))
                return false;
            else
            {
                int length = abs(intersectLine->getEndPoint().y - intersectLine->getStartPoint().y);

                intersectionRect.x = intersectLine->getStartPoint().x;
                intersectionRect.x += PADDING;

                intersectionRect.y = intersectLine->getStartPoint().y;
                intersectionRect.y += PADDING;

                intersectionRect.width = 2 * PADDING;
                intersectionRect.height = length + 2 * PADDING;

                return true;
            }
        }

        int length = abs(intersectLine->getEndPoint().x - intersectLine->getStartPoint().x);

        intersectionRect.x = intersectLine->getStartPoint().x;
        intersectionRect.x += PADDING;

        intersectionRect.y = intersectLine->getStartPoint().y;
        intersectionRect.y += PADDING;

        intersectionRect.width = length + 2 * PADDING;
        intersectionRect.height = 2 * PADDING;

        return true;
    }

    return false;
}

bool Detector::areCornersClose(const Rect& box1, const Rect& box2)
{
    int dx1 = box1.br().x - box2.tl().x;
    int dy1 = box1.br().y - box2.tl().y;

    double euclidian1 = sqrt(dx1 * dx1 + dy1 * dy1);

    int dx2 = box1.tl().x - box2.br().x;
    int dy2 = box1.tl().y - box2.br().y;

    double euclidian2 = sqrt(dx2 * dx2 + dy2 * dy2);

    double minDistance = min(euclidian1, euclidian2);

    return minDistance <= PADDING;
}

bool Detector::isBoxTaken(const vector<Rect>& boxesTaken, const Rect& proposedBox)
{
    for (int i = 0; i < boxesTaken.size(); ++i)
        if (boxesTaken[i] == proposedBox)
            return true;

    return false;
}

map<int, bool> Detector::areEquivalentRegions(const Mat& source, const vector<Rect>& before, const vector<Rect>& after)
{
    map<int, bool> mergedRegions;

    for (int i = 0; i < after.size(); ++i)
    {
        for (int j = 0; j < before.size(); ++j)
            if (edgeProcessor_->boxesAreSimilar(after[i], before[j]))
                goto found;

        goto not_found;

        found:
        mergedRegions.emplace(i, false);

        goto next_loop;

        not_found:
        mergedRegions.emplace(i, true);

        next_loop:;
    }

    return mergedRegions;
}
