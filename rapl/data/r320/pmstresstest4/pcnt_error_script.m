% first run to_rms.m & msr_error_script.m

fh3=figure;
h11=plot(t_ave,pcnt_err,'>b','markersize',10,'displayname','Package Counter');
hold on;
h10=plot(t_ave,pcnt_err2,'sg','markersize',10,'displayname','Package+DRAM Counter');
L3=legend;
xlabel('Time (seconds)');
ylabel('% Error');
T3=title({"% Error = 100*[P(rms)-dP(idle)-P(counter)]/P(rms)"});
FS3=findall(gca,'-property','Fontsize');
set(FS3, "fontsize", 12, "linewidth", 2);
grid on;
FL3= findall(L3,'-property','FontSize');
set(L3,'FontSize',12,'location','northwest');

print -dpdf pcnt_error.pdf
