taskset -c 0 ./run.sh cilk-future native future-1-native.out 1 4
#taskset -c 0-1 ./run.sh future native future-2-native.out 2 8
#taskset -c 0-3 ./run.sh future native future-4-native.out 4 16
#taskset -c 0-7 ./run.sh future native future-8-native.out 8 32 
#taskset -c 0-11 ./run.sh future native future-12-native.out 12 48
#taskset -c 0-15 ./run.sh cilk-future native future-16-native.out 16 64
taskset -c 0-15 ./run.sh cilk-future native future-16-native.out 16 64
#taskset -c 0-19 ./run.sh future native future-20-native.out 20 80
#taskset -c 0-23 ./run.sh future native future-24-native.out 24 96
#taskset -c 0-27 ./run.sh future native future-28-native.out 28 112
#taskset -c 0-31 ./run.sh future native future-32-native.out 32 128
