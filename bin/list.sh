pid=$(pidof Screen)
if [ -n "$pid"  ]; then
	echo "Screen($pid) is running"
fi
pid=$(pidof Exist)
if [ -n "$pid"  ]; then
	echo "Exist($pid) is running"
fi
pid=$(pidof SolidStateDrive)
if [ -n "$pid"  ]; then
	echo "SolidStateDrive($pid) is running"
fi
pid=$(pidof Mother)
if [ -n "$pid"  ]; then
	echo "Mother($pid) is running"
fi

