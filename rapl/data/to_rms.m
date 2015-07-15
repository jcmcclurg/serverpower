load data_scope.mat
t_iac = data_scope(1:end,1);
iac = data_scope(1:end,2);
t_vac = data_scope(1:end,3);
vac = data_scope(1:end,4);

instpwr = iac.*vac;

% compute rms:
% rms(iac(i:samplerate/60Hz)) in Matlab
samplerate = 20000; %Sa/sec
freq = 60;
sa_per_cyc = samplerate/freq;
start = round(sa_per_cyc/2);
stop = length(iac)-round(sa_per_cyc/2);
irms = zeros(length(iac),1);
vrms = zeros(length(vac),1);
sqr_iac = iac.^2;
sqr_vac = vac.^2;
for i = (start+1):stop
	irms(i) = sqrt(mean(sqr_iac((i-start):(i+start))));
end
for i = (start+1):stop
    vrms(i) = sqrt(mean(sqr_vac((i-start):(i+start))));
end
prms = irms.*vrms;

%ave_rms
sa_per_ave=round(0.15*samplerate);
len_ave = round(length(iac)/sa_per_ave);
prms_ave = zeros(round(length(iac)/sa_per_ave),1);
t_ave = zeros(round(length(iac)/sa_per_ave),1);
for i=0:(length(prms_ave)-2) % -2 cause not enough elements sometimes
	prms_ave(i+1)=mean(prms((1+i*sa_per_ave):((1+i)*sa_per_ave)));
	t_ave(i+1)=t_iac((1+i)*sa_per_ave);
end
t_ave(end)=t_ave(end-1)+0.15;

%ave_instpwr
%sa_per_ave=round(0.3*samplerate);
%instpwr = iac.*vac;
%instpwr_abs=abs(instpwr);
%pinst_ave = zeros(round(length(instpwr)/sa_per_ave),1);
%pinst_abs_ave = zeros(round(length(instpwr)/sa_per_ave),1);
%t_aveInst = zeros(round(length(instpwr)/sa_per_ave),1);
%for i=0:(length(pinst_ave)-2)
%	pinst_ave(i+1)=mean(instpwr((1+i*sa_per_ave):((1+i)*sa_per_ave)));
%	t_ave(i+1)=t_iac((1+i)*sa_per_ave);
%end
%
%for i=0:(length(pinst_abs_ave)-2)
%	pinst_abs_ave(i+1)=mean(instpwr_abs((1+i*sa_per_ave):((1+i)*sa_per_ave)));
%	t_ave(i+1)=t_iac((1+i)*sa_per_ave);
%end


	%get power_gadget data
load data_pg.mat
t_pg = data_pg(1:end,1);
t_pg = data_pg(1:end,1)-t_pg(1);
p_pg = data_pg(1:end,2);
dram_pg = data_pg(1:end,3);
tot_pg = p_pg+dram_pg;
	%load data_sp.mat
	%t_sp = data_sp(1:end,1);
	%p_sp = data_sp(1:end,2);

	%p_base=prms_ave-p_pg; %need to interpolate
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

	%plot data
fh=figure;
%h2=plot(t_iac,prms,'b','displayname','RMS Power (running period window)');

h1=plot(t_ave,prms_ave,'or','markersize',5,'displayname','RMS Average Power');
hold on;
h3=plot(t_ave,p_pg_interp,'.m','markersize',5,'displayname','PKG MSR');
%h4=plot(t_pg,tot_pg,':.b','markersize',5,'displayname','PKG+DRAM MSR');
%h5=plot(t_aveInst,pinst_ave,':.m','markersize',5,'displayname','Instantaneous Average (0.1s) Power');
<<<<<<< HEAD
h6=plot(t_ave,p_base,'>b','markersize',10,'displayname','AveRMS-msrPKG');
h7=plot(t_ave,p_base2,'sg','markersize',10,'displayname','AveRMS-msrPKG-msrDRAM');
h8=plot(xlim,[mean(prms_ave(2:30)-p_pg_interp(2:30)) mean(prms_ave(end-30:end-1)-p_pg_interp(end-30:end-1))],'-b','displayname','mean of AveRMS-msrPKG');
h9=plot(xlim,[mean(prms_ave(2:30)-tot_interp(2:30)) mean(prms_ave(end-30:end-1)-tot_interp(end-30:end-1))],'-g','displayname','mean of AveRMS-msrPKG-msrDRAM');
h10=plot(t_ave,pcnt_err2,'sg','markersize',10,'displayname','pcentErr2');
h11=plot(t_ave,pcnt_err,'>b','markersize',10,'displayname','pcentErr1');
L1 = legend;
=======
h6=plot(t_ave,p_base,'>b','markersize',2,'displayname','AveRMS-msrPKG');
h7=plot(t_ave,p_base2,'sg','markersize',2,'displayname','AveRMS-msrPKG-msrDRAM');
%h8=plot(xlim,[mean(prms_ave(2:30)-p_pg_interp(2:30)) mean(prms_ave(end-30:end-1)-p_pg_interp(end-30:end-1))],'-b','displayname','mean of AveRMS-msrPKG');
%h9=plot(xlim,[mean(prms_ave(2:30)-tot_interp(2:30)) mean(prms_ave(end-30:end-1)-tot_interp(end-30:end-1))],'-g','displayname','mean of AveRMS-msrPKG-msrDRAM');
h10=plot(t_ave,pcnt_err2,'.g','markersize',5,'displayname','pcentErr2');
h11=plot(t_ave,pcnt_err,'.b','markersize',5,'displayname','pcentErr1');
L1 = legend('show');
>>>>>>> 23a97f6a4c63e3271ba7c6ac23beb83939619832
xlabel('Time (seconds)');
ylabel('Power (W)');
FS1=findall(gca,'-property','Fontsize');
set(FS1, "fontsize", 12, "linewidth", 2);
grid on;
FL1= findall(L1,'-property','FontSize');
set(L1,'FontSize',12,'location','northwest');

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

X=[prms_ave p_pg_interp tot_interp];
csvwrite('forLSfit.csv',X);
