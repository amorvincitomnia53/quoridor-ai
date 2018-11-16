THRESHOLD=${1:-`cat threshold`}
echo "Threshold set to $THRESHOLD"
echo $THRESHOLD > threshold
tar -cz game.hpp full_game.hpp negaalpha.hpp main.cpp compile.sh run.sh threshold > ../codefes.tar.gz
