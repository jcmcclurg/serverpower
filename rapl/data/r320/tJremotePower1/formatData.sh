#!/bin/bash

# get power_gadget output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' powerGadgetLog.csv > time_pg.csv #> /dev/null
cut -d, -f3 powerGadgetLog.csv > pkg_pg.csv #> /dev/null
awk -F [,] '{printf("%.5G\n",$5)}' powerGadgetLog.csv > dram_pg.csv
paste -d ',' time_pg.csv pkg_pg.csv dram_pg.csv > pg_data.csv # final output file
rm time_pg.csv pkg_pg.csv dram_pg.csv # clean up

# get powerMeasurement output data
awk -F [' '] '{printf("%.10f,%.2f,\n",$1,$2)}' powerMeasurementLog.log > powerLogDecimated.csv
awk -F [,] '{system("date -d @"$1" +%T:%N")}' powerLogDecimated.csv > powerFormated.csv
awk -F [:::,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' powerFormated.csv > timeRemote.csv #> /dev/null
cut -d, -f2 powerLogDecimated.csv > powerRemote.csv
paste -d ',' timeRemote.csv powerRemote.csv > remoteData.csv # final output file
rm powerLogDecimated.csv powerFormated.csv timeRemote.csv powerRemote.csv  # clean up

:<<COMMENT
# Should learn to do the rest in a loop somewhat like this:
LIST="$(ls calc*Data.csv)"
for i in "$LIST"; do
	echo $i
done
COMMENT

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

# get calcSetpoint3 output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' calcSet3Data.csv > time_fmd.csv #> /dev/null
cut -d, -f2 calcSet3Data.csv > freq.csv
cut -d, -f3 calcSet3Data.csv > fmd_sp.csv
cut -d, -f4 calcSet3Data.csv > frame.csv
paste -d ',' time_fmd.csv freq.csv fmd_sp.csv frame.csv > formatedData3.csv # final output file
rm time_fmd.csv freq.csv fmd_sp.csv frame.csv

# get calcSetpoint2 output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' calcSet4Data.csv > time_fmd.csv #> /dev/null
cut -d, -f2 calcSet4Data.csv > freq.csv
cut -d, -f3 calcSet4Data.csv > fmd_sp.csv
cut -d, -f4 calcSet4Data.csv > frame.csv
paste -d ',' time_fmd.csv freq.csv fmd_sp.csv frame.csv > formatedData4.csv # final output file
rm time_fmd.csv freq.csv fmd_sp.csv frame.csv
