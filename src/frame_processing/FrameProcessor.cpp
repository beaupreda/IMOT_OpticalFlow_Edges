#include "FrameProcessor.h"

using namespace cv;
using namespace std;

FrameProcessor::FrameProcessor()
{
}

bool FrameProcessor::checkBoxesDimensions(const Rect& box1, const Rect& box2) const
{
    return !(box1.height <= 1 || box1.width <= 1 || box2.height <= 1 || box2.width <= 1);
}

bool FrameProcessor::angleIsValid(const int angle)
{
    return !((angle >= Q1_LOW && angle <= Q1_HIGH) ||
             (angle >= Q2_LOW && angle <= Q2_HIGH) ||
             (angle >= Q3_LOW && angle <= Q3_HIGH) ||
             (angle >= Q4_LOW && angle <= Q4_HIGH));
}

int FrameProcessor::computeAngleBoxes(const Rect& box1, const Rect& box2)
{
    Point2i centerBox1 = Point2i(box1.x + box1.width / 2, box1.y + box1.height / 2);
    Point2i centerBox2 = Point2i(box2.x + box2.width / 2, box2.y + box2.height / 2);

    // converts back to degrees
    int angle = static_cast<int>(atan2(centerBox1.y - centerBox2.y, centerBox1.x - centerBox2.x) * 180 / CV_PI);

    return angle;
}

int FrameProcessor::computeDistanceBoxes(const Rect& box1, const Rect& box2, const Mat& image)
{
    shared_ptr<Line> leftLineBox1 (new Line(Point2i(box1.x, box1.y), Point2i(box1.x, box1.y + box1.height), X));
    shared_ptr<Line> topLineBox1 (new Line(Point2i(box1.x, box1.y), Point2i(box1.x + box1.width, box1.y), Y));
    shared_ptr<Line> rightLineBox1 (new Line(Point2i(box1.x + box1.width, box1.y),
                                              Point2i(box1.x + box1.width, box1.y + box1.height), X));
    shared_ptr<Line> bottomLineBox1 (new Line(Point2i(box1.x, box1.y + box1.height),
                                               Point2i(box1.x + box1.width, box1.y + box1.height), Y));

    vector<shared_ptr<Line>> linesBox1 = {leftLineBox1, topLineBox1, rightLineBox1, bottomLineBox1};

    shared_ptr<Line> leftLineBox2 (new Line(Point2i(box2.x, box2.y), Point2i(box2.x, box2.y + box2.height), X));
    shared_ptr<Line> topLineBox2 (new Line(Point2i(box2.x, box2.y), Point2i(box2.x + box2.width, box2.y), Y));
    shared_ptr<Line> rightLineBox2 (new Line(Point2i(box2.x + box2.width, box2.y),
                                                Point2i(box2.x + box2.width, box2.y + box2.height), X));
    shared_ptr<Line> bottomLineBox2 (new Line(Point2i(box2.x, box2.y + box2.height),
                                                 Point2i(box2.x + box2.width, box2.y + box2.height), Y));

    vector<shared_ptr<Line>> linesBox2 = {leftLineBox2, topLineBox2, rightLineBox2, bottomLineBox2};

    // for each line in the boxes, compute orthogonal distance between parallel lines if they have an intersection
    int distance = INT32_MAX;

    for (int i = 0; i < linesBox1.size(); ++i)
    {
        for (int j = 0; j < linesBox2.size(); ++j)
        {
            if (linesBox1[i]->getConstantDirection() == linesBox2[j]->getConstantDirection() &&
                linesBox1[i]->hasIntersection(linesBox2[j], linesBox2[j]->getConstantDirection(), image))
            {
                int min = linesBox1[i]->computeDistance(linesBox2[j], linesBox2[j]->getConstantDirection());

                if (min < distance)
                    distance = min;
            }
        }
    }

    return distance;
}

int FrameProcessor::computeDistanceBlobs(const vector<Point2i>& region1, const vector<Point2i>& region2)
{
    int minimumDistance = INT32_MAX;

    for (int i = 0; i < region1.size(); ++i)
    {
        for (int j = 0; j < region2.size(); ++j)
        {
            int dx = region1[i].x - region2[j].x;
            int dy = region1[i].y - region2[j].y;

            int currentdistance = static_cast<int>(sqrt(dx * dx + dy * dy));

            if (currentdistance  < minimumDistance)
                minimumDistance = currentdistance;
        }
    }

    return minimumDistance;
}

Mat FrameProcessor::createBackgroundImage(shared_ptr<Place> place, shared_ptr<FileOperations> fileOperator)
{
    int maxFrame = 0;
    int minFrame = 0;

    if (place->getFolder() == "rene/")
    {
        maxFrame = MAX_RENE_FRAME;
        minFrame = MIN_RENE_FRAME;
    }
    else if (place->getFolder() == "rouen/")
    {
        maxFrame = MAX_ROUEN_FRAME;
        minFrame = MIN_ROUEN_FRAME;
    }
    else if (place->getFolder() == "sherbrooke/")
    {
        maxFrame = MAX_SHERBROOKE_FRAME;
        minFrame = MIN_SHERBROOKE_FRAME;
    }
    else if (place->getFolder() == "stmarc")
    {
        maxFrame = MAX_STMARC_FRAME;
        minFrame = MIN_STMARC_FRAME;
    }

    string sourceFilename = RELATIVE_PATH + place->getFolder() + place->getSubFolder() +
                            fileOperator->intToString(minFrame) + EXTENSION_JPG;

    Mat sourceImage = imread(sourceFilename, CV_LOAD_IMAGE_COLOR);

    Mat averageImage = Mat::zeros(sourceImage.size(), CV_32FC3);

    Mat scaledImage;

    for (int i = minFrame+ 1; i < maxFrame; ++i)
    {
        sourceFilename = RELATIVE_PATH + place->getFolder() + place->getSubFolder() +
                         fileOperator->intToString(i) + EXTENSION_JPG;

        sourceImage = imread(sourceFilename, CV_LOAD_IMAGE_COLOR);

        accumulateWeighted(sourceImage, averageImage, ALPHA);

        convertScaleAbs(averageImage, scaledImage);
    }

    return scaledImage;
}

vector<Rect> FrameProcessor::eliminateBoxesIncluded(const vector<Rect>& boxes)
{
    set<int> indexesToDelete;
    set<int> allIndexes;

    for (int i = 0; i < boxes.size(); ++i)
    {
        allIndexes.emplace(i);

        for (int j = i + 1; j < boxes.size(); ++j)
        {
            int intersection = (boxes[i] & boxes[j]).area();

            if (intersection > 0)
            {
                if (boxes[i].area() == intersection)
                    indexesToDelete.emplace(i);

                else if (boxes[j].area() == intersection)
                    indexesToDelete.emplace(j);
            }
        }
    }

    set<int> indexes;
    auto iterator = set_difference(allIndexes.begin(), allIndexes.end(), indexesToDelete.begin(),
                                   indexesToDelete.end(), inserter(indexes, indexes.end()));

    vector<Rect> rectangles;

    for (set<int>::iterator it = indexes.begin(); it != indexes.end(); ++it)
        rectangles.push_back(boxes[*it]);

    return rectangles;
}

vector<Rect> FrameProcessor::eliminateSmallBoxes(const vector<Rect>& boxes)
{
    int maxArea = INT32_MIN;

    for (int i = 0; i < boxes.size(); ++i)
    {
        if (boxes[i].area() > maxArea)
            maxArea= boxes[i].area();
    }

    vector<cv::Rect> acceptedBoxes;

    for (int i = 0; i < boxes.size(); ++i)
    {
        double ratio = static_cast<double>(boxes[i].area()) / static_cast<double>(maxArea);

        if (ratio >= CUTOFF)
            acceptedBoxes.push_back(boxes[i]);
    }
    return acceptedBoxes;
}

vector<Rect> FrameProcessor::eliminateCloseRegions(const vector<Rect>& boxes)
{
    map<int, int> indexes;
    vector<cv::Rect> boxesKept;

    for (int i = 0; i < boxes.size(); ++i)
    {
        for (int j = i + 1; j < boxes.size(); ++j)
        {
            if (boxesAreSimilar(boxes[i], boxes[j]))
                indexes.emplace(i, j);
        }
    }

    boxesKept = boxes;

    if (!indexes.empty())
    {
        for (auto it = indexes.begin(); it != indexes.end(); ++it)
        {
            boxesKept[it->second] = boxesKept.back();
            boxesKept.pop_back();
        }
    }

    return boxesKept;
}

void FrameProcessor::eliminateZeroBoxes(vector<Rect>& boxes)
{
    set<int> indexesToDelete;
    set<int> allIndexes;

    vector<Rect> temp = boxes;

    for (int i = 0; i < boxes.size(); ++i)
    {
        allIndexes.emplace(i);

        if (boxes[i] == Rect(0, 0, 0, 0))
            indexesToDelete.emplace(i);
    }

    set<int> indexes;

    auto iterator = set_difference(allIndexes.begin(), allIndexes.end(), indexesToDelete.begin(),
                                   indexesToDelete.end(), inserter(indexes, indexes.end()));

    boxes.resize(indexes.size());

    int counter = 0;

    for (set<int>::iterator it = indexes.begin(); it != indexes.end(); ++it)
    {
        boxes[counter] = temp[*it];
        counter++;
    }
}

vector<vector<Point>> FrameProcessor::extractContours(const Mat& binaryImage)
{
    assert(binaryImage.type() == CV_8UC1);

    vector<Vec4i> hierarchy;
    vector<vector<Point>> contours;

    findContours(binaryImage, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    return contours;
}

vector<Rect> FrameProcessor::extractInterestRegions(const Mat& binaryImage)
{
    assert(binaryImage.type() == CV_8UC1);

    vector<Vec4i> hierarchy;
    vector<vector<Point>> contours;

    findContours(binaryImage, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    vector<Rect> boundingBoxes(contours.size());

    for (int i = 0; i < contours.size(); ++i)
        boundingBoxes[i] = boundingRect(Mat(contours[i]));

    return boundingBoxes;
}

vector<Rect> FrameProcessor::extractBoxes(vector<Rect> boxes)
{
    if (boxes.empty())
    {
        Rect rect(0, 0, 0, 0);
        boxes.push_back(rect);
        return boxes;
    }

    for (int i = 0; i < boxes.size(); ++i)
    {
        for (int j = i + 1; j < boxes.size(); ++j)
        {
            int intersectArea = (boxes[i] & boxes[j]).area();

            Rect smallestBox = findSmallestBox(boxes[i], boxes[j]);

            double ratio = static_cast<double>(intersectArea) / static_cast<double>(smallestBox.area());

            if (ratio >= THRESHOLD)
            {
                Rect unionRect =  boxes[i] | boxes[j];

                boxes[i] = Rect(0, 0, 0, 0);
                boxes[j] = Rect(0, 0, 0, 0);
                boxes.push_back(unionRect);

                eliminateZeroBoxes(boxes);

                boxes = extractBoxes(boxes);
            }
        }
    }

    return boxes;
}

Point2i FrameProcessor::findTopLeftPoint(const vector<Point2i>& points)
{
    int x = INT32_MAX;
    int y = INT32_MAX;

    for (int i = 0; i < points.size(); ++i)
    {
        int currentX = points[i].x;
        int currentY = points[i].y;

        if (currentX < x)
            x = currentX;

        if (currentY< y)
            y = currentY;
    }

    return Point2i(x, y);
}

Point2i FrameProcessor::findBottomRightPoint(const vector<Point2i>& points)
{
    int x = INT32_MIN;
    int y = INT32_MIN;

    for (int i = 0; i < points.size(); ++i)
    {
        int currentX = points[i].x;
        int currentY = points[i].y;

        if (currentX > x)
            x = currentX;

        if (currentY > y)
            y = currentY;
    }

    return Point2i(x, y);
}

Point2i FrameProcessor::findCenterPosition(const vector<Point2i>& points)
{
    int sumX = 0;
    int sumY = 0;

    for (int i = 0; i < points.size(); ++i)
    {
        sumX += points[i].x;
        sumY += points[i].y;
    }

    int meanX = static_cast<int>(sumX / points.size());
    int meanY = static_cast<int>(sumY / points.size());

    return Point2i(meanX, meanY);
}

Rect FrameProcessor::findSmallestBox(const Rect& box1, const Rect& box2)
{
    int minArea = min(box1.area(), box2.area());

    if (minArea== box1.area())
        return box1;
    else
        return box2;
}

bool FrameProcessor::boxesAreSimilar(const Rect& box1, const Rect& box2)
{
    Rect intersectRectangle = box1 & box2;

    // to check if the size of the rectangles are similar
    double fractionRect1 = static_cast<double>(intersectRectangle.area()) / static_cast<double>(box1.area());
    double fractionRect2 = static_cast<double>(intersectRectangle.area()) / static_cast<double>(box2.area());

    bool xIsSimilar = false;
    bool yIsSimilar = false;

    if (fractionRect1 >= SIMILARITY && fractionRect2 >= SIMILARITY)
    {
        // to check if the 2 rectangles are close to each other
        int smallestX = min(box1.x, box2.x);
        int smallestY = min(box1.y, box2.y);

        int biggestX = max(box1.x, box2.x);
        int biggestY = max(box1.y, box2.y);

        for (int i = 0; i < PIXEL_OFFSET; ++i)
        {
            if (smallestX == biggestX)
                xIsSimilar = true;

            if (smallestY == biggestY)
                yIsSimilar = true;

            smallestX++;
            smallestY++;
        }
    }

    return xIsSimilar && yIsSimilar;
}

bool FrameProcessor::boxIsIncluded(const Rect& box1, const Rect& box2)
{
    return box1.x >= box2.x &&
           box1.y >= box2.y &&
           box1.br().x <= box2.br().x &&
           box1.br().y <= box2.br().y;
}

double FrameProcessor::intersectionOverUnion(const Rect& box1, const Rect& box2)
{
    int intersectionArea = (box1 & box2).area();
    int unionArea = box1.area() + box2.area() - intersectionArea;

    if (intersectionArea == 0)
        return 0.0;
    else
        return static_cast<double>(intersectionArea)/ static_cast<double>(unionArea);
}

double FrameProcessor::intersectionSmallestBox(const Rect &box1, const Rect &box2)
{
    int intersectArea = (box1 & box2).area();

    return max(static_cast<double>(intersectArea) / static_cast<double>(box1.area()),
               static_cast<double>(intersectArea) / static_cast<double>(box2.area()));
}


