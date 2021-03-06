#!/bin/bash
dir="/home/powerserver/joe/serverpower"
# get power_gadget output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' $dir/rapl/data/data.csv > time_pg.csv #> /dev/null
cut -d, -f3 $dir/rapl/data/data.csv > pkg_pg.csv #> /dev/null
awk -F [,] '{printf("%.5G\n",$5)}' $dir/rapl/data/data.csv > dram_pg.csv
paste -d ',' time_pg.csv pkg_pg.csv dram_pg.csv > pg_data.csv # final output file
rm time_pg.csv pkg_pg.csv dram_pg.csv # clean up

# get powerMeasurement output data
awk -F [,] '{printf("%.10f,%.3f,\n",$2,$5)}' powerMeasure.csv > powerLogDecimated.csv
awk -F [,] '{system("date -d @"$1" +%T:%N")}' powerLogDecimated.csv > powerFormated.csv
awk -F [:::,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' powerFormated.csv > timeRemote.csv #> /dev/null
cut -d, -f2 powerLogDecimated.csv > powerRemote.csv
paste -d ',' timeRemote.csv powerRemote.csv > remoteData.csv # final output file
rm powerLogDecimated.csv powerFormated.csv timeRemote.csv powerRemote.csv formatData.sh~  # clean up

# get calcSetpoint1 output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' calcSet1Data.csv > time_fmd1.csv #> /dev/null
cut -d, -f2 calcSet1Data.csv > freq1.csv
cut -d, -f3 calcSet1Data.csv > fmd_sp1.csv
cut -d, -f4 calcSet1Data.csv > frame1.csv
paste -d ',' time_fmd1.csv freq1.csv fmd_sp1.csv frame1.csv > formatedData1.csv # final output file
rm time_fmd1.csv freq1.csv fmd_sp1.csv frame1.csv # clean up
