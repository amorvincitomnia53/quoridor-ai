THRESHOLD=${1:-`cat threshold`}
echo "Threshold set to $THRESHOLD"
g++ main.cpp -std=gnu++17 -Ofast -DNDEBUG -march=native -DNOVERBOSE -DTHRESHOLD=${THRESHOLD} -o main$THRESHOLD
