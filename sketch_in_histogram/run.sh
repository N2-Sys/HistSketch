exe="./exp"

command="$exe $*"

cd build && make && cd ../bin && echo ""

if [ $? == 0 ]; then
	$command
fi
