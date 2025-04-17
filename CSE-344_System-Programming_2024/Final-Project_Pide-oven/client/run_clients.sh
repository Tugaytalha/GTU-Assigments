#!/bin/bash

server_ip="127.0.0.1"  # Change this to your server's IP address

for i in {1..10}; do
    ./client $i $((100 + $i)) $server_ip &
done

wait
