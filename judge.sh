cd build
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


./$1 input-pipe1 output-pipe1&
./$2 input-pipe2 output-pipe2&
./judge output-pipe1 input-pipe1 output-pipe2 input-pipe2
