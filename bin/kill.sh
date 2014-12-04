if [ $# == 0 ]; then
		pid=$(pidof Screen)
		if [ -n "$pid"  ]; then
			kill $pid
			echo "kill Screen $pid"
		fi
		pid=$(pidof Exist)
		if [ -n "$pid"  ]; then
			kill $pid
			echo "kill Exist $pid"
		fi
		pid=$(pidof SolidStateDrive)
		if [ -n "$pid"  ]; then
			kill $pid
			echo "kill SolidStateDrive $pid"
		fi
		pid=$(pidof Mother)
		if [ -n "$pid"  ]; then
			kill $pid
			echo "kill Mother $pid"
		fi
        exit 1;
fi

if [ $# != 1 ]; then
        echo commond:kill.sh [binfile];
        exit 1;
fi
exe=$1
echo $exe

pid=$(pidof $exe)
if [ -n "$pid"  ]; then
	kill $pid
	echo "kill $exe $pid"
fi
