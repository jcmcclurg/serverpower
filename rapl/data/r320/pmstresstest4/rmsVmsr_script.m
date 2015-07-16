% first run to_rms.m and msr_error_script.m

fh2=figure;
ha=plot(p_pg_interp,prms_ave,'>b','markersize',10,'displayname','Package Counter');
hold on;
hb=plot(tot_interp,prms_ave,'sg','markersize',10,'displayname','Package+DRAM Counter');
L2 = legend;
xlabel('MSR Energy Counter Processor Power Estimate (W)');
ylabel('Measured Server Power (rms) (W)');
FS2=findall(gca,'-property','Fontsize');
set(FS2, "fontsize", 12, "linewidth", 2);
grid on;
T2 = title({"Power (rms) vs Power (counter)"});
FL2= findall(L2,'-property','FontSize');
set(L2,'FontSize',12,'location','northwest');

print -dpdf rmsVmsr.pdf
