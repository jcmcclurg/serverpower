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
data=textscan(fid,'%f %f %f %f %f %f %f','Delimiter',',','HeaderLines',1);
fclose(fid);
fid = fopen('setpt.csv');
data2=textscan(fid,'%f %f','Delimiter',',','HeaderLines',0);
fclose(fid);
% save data as readable variables
t_lv=data{1};
p_lv=data{2}*200.; % I*20V*10(gain-scaling)
t_pg=data{3};
p_pg=data{4};
dram_pg=data{5};
tmp0_pg=data{6};
t_sp=data2{1};
p_sp=data2{2};
data_pg = [t_pg p_pg dram_pg];
data_sp = [t_sp p_sp];
save data_pg.mat data_pg;
save data_sp.mat data_sp;
%plot data
fh=figure();

plot(t_pg,p_pg,'r:.','displayname','power registers');
hold on;
grid on;
%plot(t_lv,p_lv,'b:.','displayname','power meter');
[xst,yst]=stairs(t_sp(1:end-1),p_sp(1:end-1),'g--','displayname','set point');
plot(xst,yst,'g--','displayname','setpoint');

%plot(t_lv,p_lv_ave,'g','displayname','power meter ave (over 5)');
%plot(t_lv,p_lv_ave_ten-10,'g:.','displayname','power meter ave (over 10) - 10W');
title('Estimated (processor counters) Power & Setpoint');
xlabel('Time (seconds since 00:00 hours)');
ylabel('Power (watts)');
legend();
%PrettyPlot(fh);
saveas(fh,"read_pwr.pdf");
