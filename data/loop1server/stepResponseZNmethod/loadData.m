%load_data.m
clear all; close all; clc;

if (exist('data_pg.mat','file')>0)
	load data_pg.mat;
	t_pg=data_pg(1:end,1);
	pkg_pg=data_pg(1:end,2);
	dram_pg=data_pg(1:end,3);
else

	t_pg=NaN;
	pkg_pg=NaN;
	dram_pg=NaN;
end

if (exist('data_rm.mat','file')>0)
	load data_rm.mat;
	t_rm=data_rm(1:end,1);
	p_rm=data_rm(1:end,2);
else 

	t_rm=NaN;
	p_rm=NaN;
end

load data_fmd1.mat;
t_fmd1=data_fmd1(1:end,1);
freq_fmd1=data_fmd1(1:end,2);
sp_fmd1=data_fmd1(1:end,3);
frame_fmd1=data_fmd1(1:end,4);
