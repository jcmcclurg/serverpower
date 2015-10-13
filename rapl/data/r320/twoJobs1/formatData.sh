#!/bin/bash

# get power_gadget output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' data.csv > time_pg.csv #> /dev/null
cut -d, -f3 data.csv > pkg_pg.csv #> /dev/null
awk -F [,] '{printf("%.5G\n",$5)}' data.csv > dram_pg.csv
paste -d ',' time_pg.csv pkg_pg.csv dram_pg.csv > pg_data.csv # final output file
rm time_pg.csv pkg_pg.csv dram_pg.csv # clean up

# get calcSetpoint1 output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' calcSet1Data.csv > time_fmd1.csv #> /dev/null
cut -d, -f2 calcSet1Data.csv > freq1.csv
cut -d, -f3 calcSet1Data.csv > fmd_sp1.csv
cut -d, -f4 calcSet1Data.csv > frame1.csv
paste -d ',' time_fmd1.csv freq1.csv fmd_sp1.csv frame1.csv > formatedData1.csv # final output file
rm time_fmd1.csv freq1.csv fmd_sp1.csv frame1.csv # clean up

# get calcSetpoint2 output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' calcSet2Data.csv > time_fmd2.csv #> /dev/null
cut -d, -f2 calcSet2Data.csv > freq2.csv
cut -d, -f3 calcSet2Data.csv > fmd_sp2.csv
cut -d, -f4 calcSet2Data.csv > frame2.csv
paste -d ',' time_fmd2.csv freq2.csv fmd_sp2.csv frame2.csv > formatedData2.csv # final output file
rm time_fmd2.csv freq2.csv fmd_sp2.csv frame2.csv
