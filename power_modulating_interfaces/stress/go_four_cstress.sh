n=500000
dir=$(dirname $0)
s=$dir/cstress
tee >($s $n) >($s $n) >($s $n) >($s $n) > /dev/null
