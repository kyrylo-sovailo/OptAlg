# OptAlg
Code submission for Optimization Algorithms course

### Requirements
 - [CMake](https://cmake.org/)
 - [wxWidgets](https://docs.wxwidgets.org)

### Compilation
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Usage
```
cd build
./optalg_gui # Launch GUI

./optalg_cmd --method greedy --metric area|max_size|min_size \
    --box_size 10 --item_number 100 --item_size_min 1 --item_size_max 5 \
    --loglevel 1 --seed 0 # Launch CLI greedy algorithm

./optalg_cmd --method neighborhood --neighborhood geometry|order|geometry-overlap \
    --box_size 10 --item_number 100 --item_size_min 1 --item_size_max 5 \
    --loglevel 1 --seed 0 # Launch CLI local search algorithm
```