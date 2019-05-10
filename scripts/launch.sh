# Script to run multiple servers in the background.
# Run from Mencius folder, otherwise relative paths won't work.

echo "Starting server on port 13"
./bin/server 13 &
SERVER_1_PID=$!

echo "Starting server on port 14"
./bin/server 14 &
SERVER_2_PID=$!

echo "Starting server on port 15"
./bin/server 15 &
SERVER_3_PID=$!

sleep 5

kill $SERVER_1_PID
echo "Killed server on port 13"
kill $SERVER_2_PID
echo "Killed server on port 14..."
kill $SERVER_3_PID
echo "Killed server on port 15..."
