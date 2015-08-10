%plot-data.m

top = min([length(t_log0) length(t_log1) length(t_log2) length(t_log3) length(t_log4)]);
fps_ave=(fps_log0(1:top)+fps_log1(1:top)+fps_log2(1:top)+fps_log3(1:top)+fps_log4(1:top))/5;

fh1=figure;
h_pkg=plot(t_pg,pkg_pg,'or','markersize',10,'displayname','PKG');
hold on;
[ax_yy,h_sp,h_fps]=plotyy(t_fmd,sp_fmd,t_log0,fps_ave);
set(h_sp, 'linestyle','--');
ax_sp=ax_yy(1);
ax_fps=ax_yy(2);
axes(ax_sp);
xlabel('Time (seconds)');
ylabel(ax_sp,'Power (watts)');
ylabel(ax_fps,'frames per second');
grid on;
T1 = title({"Video Transcoding DR with Frequency Deviation as Input"});
PrettyPlot;


%plot(t_pg,pkg_pg,'r');
%hold on;
%plotyy(t_fmd,sp_fmd,t_fmd,freq_fmd/1000-60);

