echo "16" | tee bintree-unstructured_result.txt bintree-spawn_result.txt
for i in $(seq 1 10); do
taskset -c 0-15 ./merge-unstructured -s1 8000000 -s2 4000000 | tee -a bintree-unstructured_result.txt
done
for i in $(seq 1 10); do
taskset -c 0-15 ./merge-spawn -s1 8000000 -s2 4000000 | tee -a bintree-spawn_result.txt
done
echo "12" | tee -a bintree-unstructured_result.txt bintree-spawn_result.txt
for i in $(seq 1 10); do
taskset -c 0-11 ./merge-unstructured -s1 8000000 -s2 4000000 | tee -a bintree-unstructured_result.txt
done
for i in $(seq 1 10); do
taskset -c 0-11 ./merge-spawn -s1 8000000 -s2 4000000 | tee -a bintree-spawn_result.txt
done
echo "8" | tee -a bintree-unstructured_result.txt bintree-spawn_result.txt
for i in $(seq 1 10); do
taskset -c 0-7 ./merge-unstructured -s1 8000000 -s2 4000000 | tee -a bintree-unstructured_result.txt
done
for i in $(seq 1 10); do
taskset -c 0-7 ./merge-spawn -s1 8000000 -s2 4000000 | tee -a bintree-spawn_result.txt
done
echo "4" | tee -a bintree-unstructured_result.txt bintree-spawn_result.txt
for i in $(seq 1 10); do
taskset -c 0-3 ./merge-unstructured -s1 8000000 -s2 4000000 | tee -a bintree-unstructured_result.txt
done
for i in $(seq 1 10); do
taskset -c 0-3 ./merge-spawn -s1 8000000 -s2 4000000 | tee -a bintree-spawn_result.txt
done
echo "2" | tee -a bintree-unstructured_result.txt bintree-spawn_result.txt
for i in $(seq 1 10); do
taskset -c 0-1 ./merge-unstructured -s1 8000000 -s2 4000000 | tee -a bintree-unstructured_result.txt
done
for i in $(seq 1 10); do
taskset -c 0-1 ./merge-spawn -s1 8000000 -s2 4000000 | tee -a bintree-spawn_result.txt
done
echo "1" | tee -a bintree-unstructured_result.txt bintree-spawn_result.txt
for i in $(seq 1 10); do
taskset -c 0 ./merge-unstructured -s1 8000000 -s2 4000000 | tee -a bintree-unstructured_result.txt
done
for i in $(seq 1 10); do
taskset -c 0 ./merge-spawn -s1 8000000 -s2 4000000 | tee -a bintree-spawn_result.txt
done
