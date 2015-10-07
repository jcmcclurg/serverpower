%load_data.m
clear all; close all; clc;

load data_pg.mat;
t_pg=data_pg(1:end,1);
pkg_pg=data_pg(1:end,2);

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
