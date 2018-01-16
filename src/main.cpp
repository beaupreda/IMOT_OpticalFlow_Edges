#include <sys/wait.h>
#include <zconf.h>

#include "detection/Detector.h"

using namespace cv;
using namespace std;

int main()
{
    shared_ptr<Detector> detector (new Detector());
    shared_ptr<FileOperations> fileOperator (new FileOperations());

    shared_ptr<Place> place (new Place("rouen/", "frames/", "backgrounds/", "mask.png", BG_OFFSET_ZERO));

    Mat backgroundImage = detector->getEdgeProcessor()->createBackgroundImage(place, fileOperator);

    place->addBackgroundImage(backgroundImage);

    string maskLocation = "../dataset/" + place->getFolder() + place->getMaskName();
    place->setMask(imread(maskLocation, CV_LOAD_IMAGE_UNCHANGED));

    string inputFile = "../dataset/" + place->getFolder() + "input.txt";

    fileOperator->parseInputFile(inputFile);

    vector<string> sourceNames, backgroundNames;

    // calls OF_DIS algorithm (creates a new process for all executions)
    for (int i = 0; i < fileOperator->getInputArguments().size(); ++i)
    {
        cout << endl << "Images OF_DIS processed = " << i + 1 << "/" <<
                fileOperator->getInputArguments().size() << endl << endl;

        sourceNames.push_back(fileOperator->getInputArguments()[i]->getPathImage1());
        backgroundNames.push_back(fileOperator->getInputArguments()[i]->getPathBackgroundImage());

        pid_t pid = fork();

        if (pid == 0)
        {
            execlp(fileOperator->getInputArguments()[i]->getPathProgram().c_str(),
                   fileOperator->getInputArguments()[i]->getProgramName().c_str(),
                   fileOperator->getInputArguments()[i]->getPathImage1().c_str(),
                   fileOperator->getInputArguments()[i]->getPathImage2().c_str(),
                   fileOperator->getInputArguments()[i]->getOutName().c_str(), NULL);

            cerr << "Exec failed for process: " << pid << endl;

            exit(0);
        }
        else
        {
            wait(NULL);
        }
    }

    fileOperator->retrieveFlowFiles("./", sourceNames.size());

    // main loop that will process every image
    for (int i = 0; i < fileOperator->getFlowFilesName().size(); ++i)
    {
        int frameNumber = fileOperator->getFileNumber(sourceNames[i]);

        Mat newBackground = detector->processFrame(place, frameNumber, fileOperator->getFlowFilesName()[i],
                                                   sourceNames[i], backgroundNames[i]);

        cout << endl << "Images processed = " << i + 1 << "/" << fileOperator->getFlowFilesName().size() << endl;

        remove(fileOperator->getFlowFilesName()[i].c_str());

        string strImageNumber = fileOperator->intToString(frameNumber);
        string newImageName = "../results/" + strImageNumber + ".png";

        vector<int> compressionParameters;
        int l = IMWRITE_PNG_BILEVEL;

        compressionParameters.push_back(l);
        compressionParameters.push_back(9);

        imwrite(newImageName, newBackground);
    }

    return 0;
}