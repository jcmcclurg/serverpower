%load_data.m

load data_pg.mat;
t_pg=data_pg(1:end,1);
pkg_pg=data_pg(1:end,2);
dram_pg=data_pg(1:end,3);

load data_fmd.mat;
t_fmd=data_fmd(1:end,1);
freq_fmd=data_fmd(1:end,2);
sp_fmd=data_fmd(1:end,3);
frame_fmd=data_fmd(1:end,4);
