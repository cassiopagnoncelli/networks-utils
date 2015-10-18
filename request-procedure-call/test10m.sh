porta=63001
./servidor 1 10 localhost &
sleep 1 
./servidor 2 10 localhost &
sleep 1
./servidor 3 10 localhost &
sleep 3
./servidor 4 10 localhost &
sleep 3
./servidor 5 10 localhost &
sleep 3
./servidor 6 10 localhost &
sleep 3
./servidor 7 10 localhost &
sleep 3
./servidor 8 10 localhost &
sleep 3
./servidor 9 10 localhost &
sleep 3
./servidor 10 10 localhost &
sleep 3
./cliente localhost $1 256+8
sleep 1
killall servidor
sleep 1
