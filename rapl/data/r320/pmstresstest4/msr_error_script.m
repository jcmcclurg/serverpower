% first run to_rms.m

% only for pmstresstest4
t_pg=t_pg-3.320;
t_ave=t_ave(45:end-44); 
prms_ave=prms_ave(45:end-44);

% t_pg=t_pg-note;
p_pg_interp=interp1(t_pg,p_pg,t_ave);
tot_interp=interp1(t_pg,tot_pg,t_ave);
p_base=prms_ave-p_pg_interp;
p_base2=prms_ave-tot_interp;
ave_base=mean(prms_ave(end-30:end-1)-p_pg_interp(end-30:end-1));
ave_base2=mean(prms_ave(end-30:end-1)-tot_interp(end-30:end-1));
ave_base_intrpl=interp1([t_ave(2) t_ave(end-1)],[ave_base ave_base],t_ave);
ave_base2_intrpl=interp1([t_ave(2) t_ave(end-1)],[ave_base2 ave_base2],t_ave);
pcnt_err=(prms_ave-p_pg_interp-ave_base)./prms_ave*100;
pcnt_err2=(prms_ave-tot_interp-ave_base2)./prms_ave*100;

fh=figure;
h6=plot(prms_ave,p_base,'>b','markersize',10,'displayname','Package Counter');
hold on;
h7=plot(prms_ave,p_base2,'sg','markersize',10,'displayname','Package+DRAM Counters');
L1 = legend;
xlabel('Power RMS (watts)');
ylabel('Error (Watts)');
FS1=findall(gca,'-property','Fontsize');
set(FS1, "fontsize", 12, "linewidth", 2);
grid on;
FL1= findall(L1,'-property','FontSize');
set(L1,'FontSize',12,'location','northwest');
T1 = title({"Error = Power(rms) - Power(counter)"});

print -dpdf msr_error.pdf
%print -dpdflatexstandalone msr_error.pdf

