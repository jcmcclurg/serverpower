% fit error

%linfit for PKG only
%linfit=46.49341+0.985322.*p_pg_interp; % from Y=a+b*x of pmstress_4_5_polyfit.xls
%linfit=45.1159+1.041922.*p_pg_interp; % from Y=a+b*x of pmstress_4_6_cut_polyfit.xls
%linfit=47.8+0.86.*p_pg_interp; % from Y=a+b*x of pmstress4_polyfit.xls
% Best for keeping error inside of +/- 6 Watts:
linfit=45.72926+1.067675.*p_pg_interp; % from Y=a+b*x of pmstress6_polyfit.xls
%linfit=45.7856+1.018868.*p_pg_interp; % from Y=a+b*x of pmstress_4cut_6_polyfit.xls

fiterror=prms_ave-linfit;
fiterror_pcnt=100*fiterror./prms_ave;

%linfit2 for PKG+DRAM
linfit2=46.60475+0.82073.*tot_interp; % from Y=a+b*x of pmstress6_wdram_polyfit.xls
fiterror2=prms_ave-linfit2;
fiterror2_pcnt=100*fiterror2./prms_ave;


fh5=figure;
h1=plot(t_ave,prms_ave,'or','markersize',10,'displayname','RMS Average Power');
hold on;
h11=plot(t_ave,linfit,'sc','markersize',5,'displayname','linear fit = b+a*PKG');
h12=plot(t_ave,linfit2,'>b','markersize',5,'displayname','linear fit = b+a*(PKG+DRAM)');

L5 = legend;
legend boxoff;
xlabel('Time (seconds)');
ylabel('Power (watts)');
FS5=findall(gca,'-property','Fontsize');
set(FS5, "fontsize", 12, "linewidth", 2);
grid on;
T5 = title({"RMS & Linear Fit vs Time"});
FL5= findall(L5,'-property','FontSize');
set(L5,'FontSize',12,'location','northeast');

print -dpdf fitplot.pdf

f6=figure;
[ax,h13,h15]=plotyy(t_ave,fiterror,t_ave,fiterror_pcnt);
axes(ax(1));
xlabel('Time (seconds)');
ylabel(ax(1),'Error (watts)');
ylabel(ax(2),'% Error');
grid on;
T6 = title({"Linear Fit Error vs Time"});
axes(ax(2));
FS6=findall(gca,'-property','Fontsize');
set(FS6, "fontsize", 12, "linewidth", 2);
axes(ax(1));
FS6=findall(gca,'-property','Fontsize');
set(FS6, "fontsize", 12, "linewidth", 2);
set(h15,'marker','s','markersize',5,'linestyle','none');
set(h13,'marker','o','markersize',5,'linestyle','none');

print -dpdf fiterrorplot.pdf

f7=figure;
[ax,h17,h19]=plotyy(prms_ave,fiterror,prms_ave,fiterror_pcnt);
axes(ax(1));
xlabel('RMS Power (watts)');
ylabel(ax(1),'Error (watts)');
ylabel(ax(2),'% Error');
grid on;
T7 = title({"Linear Fit Error vs RMS Power"});
axes(ax(2));
FS7=findall(gca,'-property','Fontsize');
set(FS7, "fontsize", 12, "linewidth", 2);
axes(ax(1));
FS7=findall(gca,'-property','Fontsize');
set(FS7, "fontsize", 12, "linewidth", 2);
set(h19,'marker','s','markersize',5,'linestyle','none');
set(h17,'marker','o','markersize',5,'linestyle','none');
axes(ax(2));
ylim([-10 10]);
xlim([50 100]);
axes(ax(1));
xlim([50 100]);
ylim([-6 6]);

print -dpdf fiterrorVpowerplot.pdf
print -dpdflatexstandalone fiterrorVpowerplot.pdf

f8=figure;
[ax8,h21,h23]=plotyy(prms_ave,fiterror2,prms_ave,fiterror2_pcnt);
axes(ax8(1));
xlabel('RMS Power (watts)');
ylabel(ax8(1),'Error (watts)');
ylabel(ax8(2),'% Error');
grid on;
T8 = title({"Linear Fit Error (PKG+DRAM) vs RMS Power"});
axes(ax8(2));
FS8=findall(gca,'-property','Fontsize');
set(FS8, "fontsize", 12, "linewidth", 2);
axes(ax8(1));
FS8=findall(gca,'-property','Fontsize');
set(FS8, "fontsize", 12, "linewidth", 2);
set(h21,'marker','o','markersize',5,'linestyle','none');
set(h23,'marker','s','markersize',5,'linestyle','none');
axes(ax8(2));
ylim([-10 10]);
xlim([50 100]);
axes(ax8(1));
xlim([50 100]);
ylim([-6 6]);

print -dpdf fiterror2Vpowerplot.pdf
print -dpdflatexstandalone fiterror2Vpowerplot.pdf
