mkdir build -p
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

if [ ! -p input-pipe1 ]; then
    mkfifo input-pipe1
fi
if [ ! -p output-pipe1 ]; then
    mkfifo output-pipe1
fi
if [ ! -p input-pipe2 ]; then
    mkfifo input-pipe2
fi
if [ ! -p output-pipe2 ]; then
    mkfifo output-pipe2
fi

