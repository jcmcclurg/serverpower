% plotData.m
close all;

n = 7; % sample averaging window
% average pkg over window n
pkg_new = arrayfun(@(i) mean(pkg_pg(i:i+n-1)),1:n:length(pkg_pg)-n+1)';
t_new = arrayfun(@(i) t_pg(i),1:n:length(pkg_pg)-n+1)';

fh1 = figure();
h_pkg=plot(t_new,pkg_new,'--*m','markersize',10,'linewidth',4);
hold on;
[x_sp1,y_sp1]=stairs(t_fmd1,sp_fmd1);
[ax_yy,h_sp1,h_freq1]=plotyy(x_sp1,y_sp1,t_fmd1,freq_fmd1/1000-60.0);
[x_sp2,y_sp2]=stairs(t_fmd2,sp_fmd2);
h_sp2 = plot(ax_yy(1),x_sp2,y_sp2,'k','linewidth',4,'displayname','Setpoint (no deadline)');
h_freq2 = plot(ax_yy(2),t_fmd2,freq_fmd2/1000-60.0,'displayname','Freq (no deadline)');
%{
set(h_sp1, 'linestyle','--','linewidth',4);
set([h_pkg h_freq1],'linewidth',4);
ax_sp1=ax_yy1(1);
ax_freq1=ax_yy1(2);
xlabel('Time (seconds)');
ylabel(ax_sp1,'Power (watts)');
ylabel(ax_freq1,'Freq Dev. (Hz)');
grid on;
T1 = title({"Video Transcoding DR with Frequency Deviation and Buffer as Input"});
[hleg, hleg_obj, hplot, labels]=legend([h_pkg h_sp1 h_freq1],{'PKG','Setpoint','Freq Dev.'},'location','northeast');
set(hleg_obj,'linewidth',4);
FS=findall(hleg_obj,'-property','fontsize');
set(FS,'fontsize',18);
FS=findall(ax_sp1,'-property','Fontsize');
set(FS, "fontsize", 18, "linewidth", 2);
FS=findall(ax_freq1,'-property','Fontsize');
set(FS, "fontsize", 18, "linewidth", 2);
%}
legend boxoff;
hold off;

fh2=figure;
%h_pkg=plot(t_pg,pkg_pg,':r','markersize',10,'linewidth',2);
h_pkg=plot(t_new,pkg_new,'--*m','markersize',10,'linewidth',4);
hold on
[x_sp1,y_sp1]=stairs(t_fmd1,sp_fmd1);
[ax_yy,h_sp1,h_frame1]=plotyy(x_sp1,y_sp1,t_fmd1,frame_fmd1); hold on
set(h_sp1, 'linestyle','--','linewidth',4);
set([h_pkg h_frame1],'linewidth',4);
[x_sp2,y_sp2]=stairs(t_fmd2,sp_fmd2);
%{
h_sp2 = plot(ax_yy(1),x_sp2,y_sp2,'k','linewidth',4,'displayname','Setpoint (no deadlines)'); hold on
h_frame2 = plot(ax_yy(2),t_fmd2,frame_fmd2);
%h_frame2 = plot(ax_yy(2),t_fmd2,frame_fmd2,'--k','linewidth',4,'displayname','Frame# (no deadlines)');
ax_sp=ax_yy(1);
%ylim(ax_sp,[20,35]);
ax_frame=ax_yy(2);
%ylim(ax_fps,[50,150]);
%axes(ax_sp);
ylim(ax_frame,[min(min(frame_fmd1),min(frame_fmd2)) max(max(frame_fmd1),max(frame_fmd2))]);
xlabel('Time (seconds)');
ylabel(ax_sp,'Power (watts)');
ylabel(ax_frame,'Frame Number');
grid on;
T1 = title({"Video Transcoding DR with Frequency Deviation and Buffer as Input"});
[hleg, hleg_obj, hplot, labels]=legend([h_pkg h_sp1 h_frame1],{'PKG','Setpoint','Frame#'},'location','northeast');
set(hleg_obj,'linewidth',4);
FS=findall(hleg_obj,'-property','fontsize');
set(FS,'fontsize',18);
FS=findall(ax_sp,'-property','Fontsize');
set(FS, "fontsize", 18, "linewidth", 2);
FS=findall(ax_frame,'-property','Fontsize');
set(FS, "fontsize", 18, "linewidth", 2);
legend boxoff;
hold off;
%}
figure;
subplot(2,1,1);
h_pkg=plot(t_new,pkg_new,'--*m','markersize',10,'linewidth',4,'displayname','PKG Power'); hold on
hsp1 = plot(x_sp1,y_sp1,'r','displayname','Setpoint w/ deadlines','linewidth', 4);
hsp2 = plot(x_sp2,y_sp2,'--b','displayname','Setpoint w/o deadlines','linewidth',4);
legend;
subplot(2,1,2);
h_frame1 = plot(t_fmd1,frame_fmd1,'r','displayname','Frames w/ deadlines','linewidth',4); hold on
[ax_yy,h_frame2,h_freq] = plotyy(t_fmd2,frame_fmd2,t_fmd2,freq_fmd2/1000-60.0);
set(h_frame2, 'linewidth',4,'displayname','Frames w/o deadlines','linestyle','--','color','blue');
set(h_freq,'linewidth',4);
xlabel('Time (seconds)');
ylabel(ax_yy(1),'Frames in Buffer');
ylabel(ax_yy(2),'Frequency Deviation [Hz]');
legend;
