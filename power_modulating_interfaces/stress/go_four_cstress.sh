p=0.01
dir=$(dirname $0)
s=$dir/cstress
tee >($s -i 1: -p $p) >($s -i 2: -p $p) >($s -i 3: -p $p) >($s -i 4: -p $p) > /dev/null
