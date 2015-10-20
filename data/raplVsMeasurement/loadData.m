%load_data.m
clear all; close all; clc;

load data_pg.mat;
t_pg=data_pg(1:end,1);
pkg_pg=data_pg(1:end,2);
dram_pg=data_pg(1:end,3);

load data_rm.mat;
t_rm=data_rm(1:end,1);
p_rm=data_rm(1:end,2);
