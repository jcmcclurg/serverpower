%read_save_data.m
close all;
clear all;
clc;

%open power_gadget data file and get data
fid = fopen('pg_data.csv');
pg_data = textscan(fid,'%f %f %f','Delimiter',',','Headerlines',1);
fclose(fid);
% convert data and save to .mat
t_pg=pg_data{1};
pkg_pg=pg_data{2};
dram_pg=pg_data{3};
data_pg = [t_pg pkg_pg dram_pg];
save data_pg.mat data_pg;

%open powerMeasurementLog data file and get data
fid = fopen('remoteData.csv');
rm_data = textscan(fid,'%f %f','Delimiter',',','Headerlines',0);
fclose(fid);
% convert data and save to .mat
t_rm=rm_data{1};
p_rm=rm_data{2};
data_rm = [t_rm p_rm];
save data_rm.mat data_rm;
