sudo apt update -y
sudo apt-get install git -y
git --version
sudo apt update -y
sudo apt install build-essential -y
sudo apt update -y
sudo snap install cmake --classic
sudo apt-get update -y
sudo apt-get install can-utils -y
sudo modprobe vcan &&
sudo ip link add dev vcan0 type vcan &&
sudo ip link set up vcan0
sudo apt-get install libsqlite3-dev -y
sudo apt-get update -y
sudo apt-get install libbluetooth3-dev -y
sudo make main
sudo ./telematic -m 6 -f 1