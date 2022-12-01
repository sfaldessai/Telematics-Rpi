REQUIRED_PKG="can-utils gcc"
for i in $REQUIRED_PKG; do
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $i)
	echo Checking for $i:$PKG_OK
	if [ "" = "$PKG_OK" ];then
		echo "No $i.Setting up $i."
		sudo apt-get --yes install $i
	fi
done
sudo modprobe vcan &&
sudo ip link add dev vcan0 type vcan &&
sudo ip link set up vcan0
gcc src/can_bus/virtual_can_server.c -o virtualcan -lpthread
sudo ./virtualcan
