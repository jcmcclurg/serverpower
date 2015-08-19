#!/bin/bash

# get power_gadget output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' data.csv > time_pg.csv #> /dev/null
cut -d, -f3 data.csv > pkg_pg.csv #> /dev/null
awk -F [,] '{printf("%.5G\n",$5)}' data.csv > dram_pg.csv
paste -d ',' time_pg.csv pkg_pg.csv dram_pg.csv > pg_data.csv # final output file

# get fmd2server output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' test.csv > time_fmd.csv #> /dev/null
cut -d, -f2 test.csv > freq.csv
cut -d, -f3 test.csv > fmd_sp.csv
cut -d, -f4 test.csv > frame.csv
paste -d ',' time_fmd.csv freq.csv fmd_sp.csv frame.csv > fmd_data.csv # final output file


