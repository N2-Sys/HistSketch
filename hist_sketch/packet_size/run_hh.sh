exe="./exp_hh"

cd build && make && cd ../bin && echo ""

if [ $? == 0 ]; then
	$exe
fi
