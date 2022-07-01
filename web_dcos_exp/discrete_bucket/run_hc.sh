exe="./exp_hc"

command="$exe $*"

cd build && make && cd ../bin && echo ""

if [ $? == 0 ]; then
	$command
fi
