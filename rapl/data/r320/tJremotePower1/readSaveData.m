%read_save_data.m
close all;
clear all;
clc;

%open power_gadget data file and get data
fid = fopen('remoteData.csv');
pg_data = textscan(fid,'%f %f','Delimiter',',','Headerlines',0);
fclose(fid);
% convert data and save to .mat
t_pg=pg_data{1};
pkg_pg=pg_data{2};
data_pg = [t_pg pkg_pg];
save data_pg.mat data_pg;

%open fmd output data file and get data
fid = fopen('formatedData1.csv');
fmd_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',0);
fclose(fid);
% convert data and save to .mat
t_fmd=fmd_data{1};
freq_fmd=fmd_data{2};
sp_fmd=fmd_data{3};
frame_fmd=fmd_data{4};
data_fmd1 = [t_fmd freq_fmd sp_fmd frame_fmd];
save data_fmd1.mat data_fmd1;

%open fmd output data file and get data
fid = fopen('formatedData2.csv');
fmd_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',0);
fclose(fid);
% convert data and save to .mat
t_fmd=fmd_data{1};
freq_fmd=fmd_data{2};
sp_fmd=fmd_data{3};
frame_fmd=fmd_data{4};
data_fmd2 = [t_fmd freq_fmd sp_fmd frame_fmd];
save data_fmd2.mat data_fmd2;

%open fmd output data file and get data
fid = fopen('formatedData3.csv');
fmd_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',0);
fclose(fid);
% convert data and save to .mat
t_fmd=fmd_data{1};
freq_fmd=fmd_data{2};
sp_fmd=fmd_data{3};
frame_fmd=fmd_data{4};
data_fmd3 = [t_fmd freq_fmd sp_fmd frame_fmd];
save data_fmd3.mat data_fmd3;

%open fmd output data file and get data
fid = fopen('formatedData4.csv');
fmd_data = textscan(fid,'%f %f %f %f','Delimiter',',','Headerlines',0);
fclose(fid);
% convert data and save to .mat
t_fmd=fmd_data{1};
freq_fmd=fmd_data{2};
sp_fmd=fmd_data{3};
frame_fmd=fmd_data{4};
data_fmd4 = [t_fmd freq_fmd sp_fmd frame_fmd];
save data_fmd4.mat data_fmd4;
