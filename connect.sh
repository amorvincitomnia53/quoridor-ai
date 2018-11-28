cd build
if [ ! -p input-pipe1 ]; then
    mkfifo input-pipe1
fi
if [ ! -p output-pipe1 ]; then
    mkfifo output-pipe1
fi

./tcp-pipe $1 > input-pipe1 < output-pipe1 &
./$2 < input-pipe1 > output-pipe1




