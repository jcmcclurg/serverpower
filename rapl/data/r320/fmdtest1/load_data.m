%load_data.m

load data_pg.mat;
t_pg=data_pg(1:end,1);
pkg_pg=data_pg(1:end,2);
dram_pg=data_pg(1:end,3);

load data_fmd.mat;
t_fmd=data_fmd(1:end,1);
freq_fmd=data_fmd(1:end,2);
sp_fmd=data_fmd(1:end,3);

load data_log0.mat;
t_log0 = data_log0(1:end,1);
fps_log0 = data_log0(1:end,2);
frame_log0 = data_log0(1:end,3);
frate_log0 = data_log0(1:end,4);

load data_log1.mat;
t_log1 = data_log1(1:end,1);
fps_log1 = data_log1(1:end,2);
frame_log1 = data_log1(1:end,3);
frate_log1 = data_log1(1:end,4);

load data_log2.mat;
t_log2 = data_log2(1:end,1);
fps_log2 = data_log2(1:end,2);
frame_log2 = data_log2(1:end,3);
frate_log2 = data_log2(1:end,4);

load data_log3.mat;
t_log3 = data_log3(1:end,1);
fps_log3 = data_log3(1:end,2);
frame_log3 = data_log3(1:end,3);
frate_log3 = data_log3(1:end,4);

load data_log4.mat;
t_log4 = data_log4(1:end,1);
fps_log4 = data_log4(1:end,2);
frame_log4 = data_log4(1:end,3);
frate_log4 = data_log4(1:end,4);
