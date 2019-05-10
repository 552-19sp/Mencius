# Script to run multiple servers in the background.
# Run from Mencius folder, otherwise relative paths won't work.

echo "Starting server on port 11111"
./bin/server 11111 &
SERVER_1_PID=$!

echo "Starting server on port 11112"
./bin/server 11112 &
SERVER_2_PID=$!

echo "Starting server on port 11113"
./bin/server 11113 &
SERVER_3_PID=$!

sleep 30

kill $SERVER_1_PID
echo "Killed server on port 11111"
kill $SERVER_2_PID
echo "Killed server on port 11112"
kill $SERVER_3_PID
echo "Killed server on port 11113"
