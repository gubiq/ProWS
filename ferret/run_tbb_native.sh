#taskset -c 0 ./run.sh tbb native tbb-1-native.out 1 4
#taskset -c 0-1 ./run.sh tbb native tbb-2-native.out 2 8
#taskset -c 0-3 ./run.sh tbb native tbb-4-native.out 4 16
#taskset -c 0-7 ./run.sh tbb native tbb-8-native.out 8 32 
#taskset -c 0-11 ./run.sh tbb native tbb-12-native.out 12 48
taskset -c 0-15 ./run.sh tbb native tbb-16-native.out 16 64
#taskset -c 0-19 ./run.sh tbb native tbb-20-native.out 20 80
#taskset -c 0-23 ./run.sh tbb native tbb-24-native.out 24 96
#taskset -c 0-27 ./run.sh tbb native tbb-28-native.out 28 112
#taskset -c 0-31 ./run.sh tbb native tbb-32-native.out 32 128
