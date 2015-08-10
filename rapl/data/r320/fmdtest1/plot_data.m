%plot-data.m

top = min([length(t_log0) length(t_log1) length(t_log2) length(t_log3) length(t_log4)]);
fps_ave=(fps_log0(1:top)+fps_log1(1:top)+fps_log2(1:top)+fps_log3(1:top)+fps_log4(1:top))/5;


fh1=figure;
h_pkg=plot(t_pg,pkg_pg,':r','markersize',10,'linewidth',6);
hold on;
[ax_yy,h_sp,h_fps]=plotyy(t_fmd,sp_fmd,t_log0,fps_ave);
set(h_sp, 'linestyle','--','linewidth',4);
set([h_pkg h_fps],'linewidth',4);
ax_sp=ax_yy(1);
ax_fps=ax_yy(2);
axes(ax_sp);
xlabel('Time (seconds)');
ylabel(ax_sp,'Power (watts)');
ylabel(ax_fps,'frames per second');
grid on;
T1 = title({"Video Transcoding DR with Frequency Deviation as Input"});
[hleg, hleg_obj, hplot, labels]=legend([h_pkg h_sp h_fps],{'PKG','Setpoint','FPS'},'location','northeast');
set(hleg_obj,'linewidth',4);
FS=findall(hleg_obj,'-property','fontsize');
set(FS,'fontsize',18);
legend boxoff;
FS=findall(ax_sp,'-property','Fontsize');
set(FS, "fontsize", 18, "linewidth", 4);
FS=findall(ax_fps,'-property','Fontsize');
set(FS, "fontsize", 18, "linewidth", 4);



%plot(t_pg,pkg_pg,'r');
%hold on;
%plotyy(t_fmd,sp_fmd,t_fmd,freq_fmd/1000-60);

