%load data_scope.mat
%t_iac = data_scope(1:end,1);
%iac = data_scope(1:end,2);
%t_vac = data_scope(1:end,3);
%vac = data_scope(1:end,4);

instpwr = iac.*vac;

% compute rms:
% rms(iac(i:samplerate/60Hz)) in Matlab
samplerate = 10000; %Sa/sec
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
sa_per_ave=round(0.1*samplerate);
len_ave = round(length(iac)/sa_per_ave);
prms_ave = zeros(round(length(iac)/sa_per_ave),1);
t_ave = zeros(round(length(iac)/sa_per_ave),1);
for i=0:(length(prms_ave)-2) % -2 cause not enough elements sometimes
	prms_ave(i+1)=mean(prms((1+i*sa_per_ave):((1+i)*sa_per_ave)));
	t_ave(i+1)=t_iac((1+i)*sa_per_ave);
end

%ave_instpwr
sa_per_ave=round(0.1*samplerate);
instpwr = iac.*vac;
instpwr_abs=abs(instpwr);
pinst_ave = zeros(round(length(instpwr)/sa_per_ave),1);
pinst_abs_ave = zeros(round(length(instpwr)/sa_per_ave),1);
t_ave = zeros(round(length(instpwr)/sa_per_ave),1);
for i=0:(length(pinst_ave)-2)
	pinst_ave(i+1)=mean(instpwr((1+i*sa_per_ave):((1+i)*sa_per_ave)));
	t_ave(i+1)=t_iac((1+i)*sa_per_ave);
end

for i=0:(length(pinst_abs_ave)-2)
	pinst_abs_ave(i+1)=mean(instpwr_abs((1+i*sa_per_ave):((1+i)*sa_per_ave)));
	t_ave(i+1)=t_iac((1+i)*sa_per_ave);
end


	%get power_gadget data
load data_pg.mat
t_pg = data_pg(1:end,1)-60407.9;
p_pg = data_pg(1:end,2);
	%load data_sp.mat
	%t_sp = data_sp(1:end,1);
	%p_sp = data_sp(1:end,2);

	%p_base=prms_ave-p_pg; %need to interpolate
p_pg_interp=interp1(t_pg,p_pg,t_ave);
p_base=pinst_ave-p_pg_interp;

	%plot data
figure;
h2=plot(t_iac,prms,'b','displayname','RMS Power (running period window)');
hold on;
h1=plot(t_ave,prms_ave,':.r','linewidth',3,'displayname','RMS Average (0.1s) Power');
h3=plot(t_pg,p_pg,':.g','linewidth',3,'displayname','Energy Counter Power Estimate');
h4=plot(t_ave,pinst_ave,':.m','linewidth',3,'displayname','Instantaneous Average (0.1s) Power');

h6=plot(t_ave,p_base,':.k','linewidth',3,'displayname','Baseline Idle Power = InstAvePwr-EngyCntrPwr');
legend();
   

	
