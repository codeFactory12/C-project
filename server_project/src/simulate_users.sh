#!/bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT=8080
NUM_USERS=30

commands=("get info" "get number of partitions" "get current kernel version" "sshd running")

for i in $(seq 1 $NUM_USERS); do
    (
        # Randomly pick a command from the array
        cmd=${commands[$RANDOM % ${#commands[@]}]}
        echo "User $i sending command: $cmd"

        # Use netcat to send the command to the server and capture the response
        { echo "$cmd"; sleep 4; } | nc $SERVER_IP $SERVER_PORT &

    ) &
done

wait
echo "All commands sent."
