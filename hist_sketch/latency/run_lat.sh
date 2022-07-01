exe="./exp_latency"

cd build && make && cd ../bin && echo ""

if [ $? == 0 ]; then
	$exe
fi
