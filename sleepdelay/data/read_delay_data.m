%read_pwr_data.m
close all;
clear all;
clc;
%t_lv = csvread('data_4_15_15.csv','B2:B911');
%p_lv = csvread('data_4_15_15.csv','A2:A911');
%t_pg = csvread('data_4_15_15.csv','C2:C1072');
%p_pg = csvread('data_4_15_15.csv','D2:D1072');
%p_lv_ave = csvread('data_4_15_15.csv','G2:G911');
%p_lv_ave_ten = csvread('data_4_15_15.csv','H2:H911');

% open file and get data
fid = fopen('pver.csv');
data=textscan(fid,'%f %f %f %f %f %f','Delimiter',',','HeaderLines',1);
fclose(fid);
fid = fopen('dly.csv');
data2=textscan(fid,'%f %f','Delimiter',',','HeaderLines',1);
fclose(fid);
% save data as readable variables
t_lv=data{1};
p_lv=data{2}*200.; % I*20V*10(gain-scaling)
t_pg=data{3};
p_pg=data{4};
tmp0_pg=data{5};
t_dl=data2{1};
d_dl=data2{2};

%plot data
fh=figure();

plot(t_pg,p_pg,'r:.','displayname','power registers');
hold on;
grid on;
plot(t_dl,-log(d_dl),'b:.','displayname','delay time');
title('Time delay & Estimated (processor counters) Power');
xlabel('Time (seconds since 00:00 hours)');
ylabel('Power (watts)');
legend();
%PrettyPlot(fh);
saveas(fh,"read_pwr.pdf");
