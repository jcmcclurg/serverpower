%load_data.m
clear all; close all; clc;

load data_pg.mat;
t_pg=data_pg(1:end,1);
pkg_pg=data_pg(1:end,2);
dram_pg=data_pg(1:end,3);

load data_rm.mat;
t_rm=data_rm(1:end,1);
p_rm=data_rm(1:end,2);

load data_fmd1.mat;
t_fmd1=data_fmd1(1:end,1);
freq_fmd1=data_fmd1(1:end,2);
sp_fmd1=data_fmd1(1:end,3);
frame_fmd1=data_fmd1(1:end,4);

load data_fmd2.mat;
t_fmd2=data_fmd2(1:end,1);
freq_fmd2=data_fmd2(1:end,2);
sp_fmd2=data_fmd2(1:end,3);
frame_fmd2=data_fmd2(1:end,4);

load data_fmd3.mat;
t_fmd3=data_fmd3(1:end,1);
freq_fmd3=data_fmd3(1:end,2);
sp_fmd3=data_fmd3(1:end,3);
frame_fmd3=data_fmd3(1:end,4);

load data_fmd4.mat;
t_fmd4=data_fmd4(1:end,1);
freq_fmd4=data_fmd4(1:end,2);
sp_fmd4=data_fmd4(1:end,3);
frame_fmd4=data_fmd4(1:end,4);
