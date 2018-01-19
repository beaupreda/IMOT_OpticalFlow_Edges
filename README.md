# Improving Multiple Object Tracking with Optical Flow and Edge Preprocessing

## Running the demo

### Dependencies
- OpenCV
- Eigen3

### Data
You need both the frames and background subtraction images avaiable at [https://www.jpjodoin.com/urbantracker/dataset.html].
Place the frames and background subtraction images in the "frames" and "backgrounds" folders present in the dataset directory of the corresponding place.

### Compiling
From the IMOT_OpticalFlow_Edges directory:
```
mkdir build
cd build
cmake ../
make -j
```

### Demo
```
./IMOT_OpticalFlow_Edges PLACE
```
where PLACE is either:
- rene
- rouen
- sherbrooke
- stmarc

The images resulted will be placed in the "results" folder.

### Testing the results
You will need both the Urban Tracker and the Metric Tools avaiable at [https://www.jpjodoin.com/urbantracker/tools.html].
Follow the steps for the tracker, but take the background subtraction images produced by the algorithm (the ones in the results folder).

## Bugs
If you find any bugs or encounter any problem, feel free to contact me at [david-alexandre.beaupre@polymtl.ca].

## License
See the LICENSE file for more details.

