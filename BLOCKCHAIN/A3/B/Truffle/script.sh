#!/bin/bash

# Open a new terminal window and start Ganache CLI
gnome-terminal -- bash -c "ganache-cli --port 8545; exec bash" &

# Wait for Ganache CLI to start
sleep 2

# Open a new terminal window to compile, migrate, and test Truffle contracts
gnome-terminal -- bash -c "
truffle compile;
truffle migrate;
truffle test;
exec bash"

# Optionally, if you want to manually stop Ganache CLI after testing:
echo "Press any key to kill Ganache CLI and exit..."
read -n 1 -s
pkill -f 'ganache-cli'
