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

%open fmd output data file and get data
fid = fopen('fmd_data.csv');
fmd_data = textscan(fid,'%f %f %f','Delimiter',',','Headerlines',0);
fclose(fid);
% convert data and save to .mat
t_fmd=fmd_data{1};
freq_fmd=fmd_data{2};
sp_fmd=fmd_data{3};
data_fmd = [t_fmd freq_fmd sp_fmd];
save data_fmd.mat data_fmd;

%open log data and get data
fid = fopen('log.csv');
log_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',1);
fclose(fid);
fid = fopen('log1.csv');
log1_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',1);
fclose(fid);
fid = fopen('log2.csv');
log2_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',1);
fclose(fid);
fid = fopen('log3.csv');
log3_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',1);
fclose(fid);
fid = fopen('log4.csv');
log4_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',1);
fclose(fid);
% convert data and save to .mat
t_log0 = log_data{3};
fps_log0 = log_data{4};
frame_log0 = log_data{1};
frate_log0 = log_data{2};
data_log0 = [t_log0 fps_log0 frame_log0 frate_log0];
save data_log0.mat data_log0;
t_log1 = log1_data{3};
fps_log1 = log1_data{4};
frame_log1 = log1_data{1};
frate_log1 = log1_data{2};
data_log1 = [t_log1 fps_log1 frame_log1 frate_log1];
save data_log1.mat data_log1;
t_log2 = log2_data{3};
fps_log2 = log2_data{4};
frame_log2 = log2_data{1};
frate_log2 = log2_data{2};
data_log2 = [t_log2 fps_log2 frame_log2 frate_log2];
save data_log2.mat data_log2;
t_log3 = log3_data{3};
fps_log3 = log3_data{4};
frame_log3 = log3_data{1};
frate_log3 = log3_data{2};
data_log3 = [t_log3 fps_log3 frame_log3 frate_log3];
save data_log3.mat data_log3;
t_log4 = log4_data{3};
fps_log4 = log4_data{4};
frame_log4 = log4_data{1};
frate_log4 = log4_data{2};
data_log4 = [t_log4 fps_log4 frame_log4 frate_log4];
save data_log4.mat data_log4;

