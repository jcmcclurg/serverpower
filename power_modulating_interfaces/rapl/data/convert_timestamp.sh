#!/bin/bash
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' data.csv > time_pg.csv #> /dev/null
cut -d, -f3 data.csv > pkg_pg.csv #> /dev/null
awk -F [,] '{printf("%.5G\n",$5)}' data.csv > dram_pg.csv
# convert labview data to csv
awk '{printf("%s,%s\n",$1,$2)}' data_lv.txt > data_lv.csv #> /dev/null
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000))}' data_lv.csv > time_lv.csv #> /dev/null
cut -f2 data_lv.txt > power_lv.csv #> /dev/null
awk -F, '{printf("%d,\n",$6/1000)}' data.csv > thermalzns.csv #> /dev/null

awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000))}' setpoint.csv > time_sp.csv #> /dev/null
cut -d, -f2 setpoint.csv > power_sp.csv #> /dev/null

# get stress setpoints
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' pmstresstest_setpoint.csv > time_stress_sp.csv #> /dev/null
#awk -F [,] '{printf("%G,%G,\n",$2,$3)}' pmstresstest_setpoint.csv > mem_touch_sp.csv
cut -d, -f2,3 pmstresstest_setpoint.csv > mem_touch_sp.csv

# now paste together with:

paste -d ',' time_lv.csv power_lv.csv time_pg.csv pkg_pg.csv dram_pg.csv thermalzns.csv > pver.csv #> /dev/null
paste -d ',' time_sp.csv power_sp.csv > setpt.csv #> /dev/null
paste -d ',' time_stress_sp.csv mem_touch_sp.csv > stress_setpoint.csv 
#octave ~/joe/serverpower/rapl/data/read_pwr_data.m
