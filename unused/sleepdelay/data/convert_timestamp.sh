#!/bin/bash
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000))}' data.csv | tee time_pg.csv #> /dev/null
cut -d, -f3 data.csv | tee power_pg.csv #> /dev/null
# convert labview data to csv
awk '{printf("%s,%s\n",$1,$2)}' data_lv.txt | tee data_lv.csv #> /dev/null
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000))}' data_lv.csv | tee time_lv.csv #> /dev/null
cut -f2 data_lv.txt | tee power_lv.csv #> /dev/null
awk -F, '{printf("%d,\n",$6/1000)}' data.csv | tee thermalzns.csv #> /dev/null

awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000))}' setpoint.csv | tee time_sp.csv #> /dev/null
cut -d, -f2 setpoint.csv | tee power_sp.csv #> /dev/null

# convert delay data
#!/bin/bash
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000))}' delay.csv | tee time_dl.csv
cut -d, -f2 delay.csv | tee delay_dl.csv

# get work done by workers
cat worker* > work.txt

# now paste together with:

paste -d ',' time_lv.csv power_lv.csv time_pg.csv power_pg.csv thermalzns.csv > pver.csv #> /dev/null
paste -d ',' time_sp.csv power_sp.csv > setpt.csv #> /dev/null
paste -d ',' time_dl.csv delay_dl.csv > dly.csv

#octave read_pwr_data.m
