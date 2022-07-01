exe="./latencyGen"

cd build && make && cd ../bin && echo ""

if [ $? == 0 ]; then
	$exe
fi
