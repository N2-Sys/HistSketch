# Source Code of HistSketch

### Usage

+ For each kind of experiment (S-in-H, H-in-S, I-in-S), go to the relative directory
+ Run `initialize.sh` to build the programme first
+ Run `run.sh` or `run_hh.sh` to run the programme
+ The parameters are configured in `parameters.h`



### Dataset

We provide the dataset of CAIDA-2018 (already processed).

WebDocs and ISCXVPN-2016 can be downloaded from the Internet.



### Notice

To compare the basic version and the optimized version, comment the code marked as optimized version, and uncomment the code marked as basic version in `histogram.h` and `sketch.h`.