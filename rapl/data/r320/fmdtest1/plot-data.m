%plot-data.m

plot(t_pg,pkg_pg,'r');
hold on;
plotyy(t_fmd,sp_fmd,t_fmd,freq_fmd/1000-60);

