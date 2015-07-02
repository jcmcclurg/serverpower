n=500000
s=./cstress
tee >($s $n) >($s $n) >($s $n) >($s $n) > /dev/null
