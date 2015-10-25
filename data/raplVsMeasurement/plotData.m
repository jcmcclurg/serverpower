% plotData.m
clear all; clc; close all;
source("~/Documents/research/serverpower/data/raplVsMeasurement/loadData.m");

figure
plot(t_pg,pkg_pg,'.r','displayname','RAPL PKG'); hold on
%plot(t_pg,dram_pg+pkg_pg,'.b','linewidth',1,'displayname','RAPL PKG+DRAM');
plot(t_rm,p_rm,'.k','displayname','DAQ');


n = 100; % sample averaging window
% average pkg over window n
p_mean = arrayfun(@(i) mean(p_rm(i:i+n-1)),4:n:length(p_rm)-n+1)';
p_std = arrayfun(@(i) std(p_rm(i:i+n-1)),4:n:length(p_rm)-n+1)';
t_new = arrayfun(@(i) t_rm(i),4:n:length(p_rm)-n+1)';
duty = [0.001:0.01:0.5];
[tStep,pMeanStep]=stairs(duty,p_mean);
[tStep,pStdStep]=stairs(duty,p_std);


%pkg_mean = arrayfun(@(i) mean(pkg_pg(i:i+n-1)),4:n:length(pkg_pg)-n+1)';
%pkg_std = arrayfun(@(i) std(pkg_pg(i:i+n-1)),4:n:length(pkg_pg)-n+1)';
%t_new_pg = arrayfun(@(i) t_pg(i),4:n:length(pkg_pg)-n+1)';
%[tPkgStep,pkgMeanStep]=stairs(duty,pkg_mean);
%[tPkgStep,pkgStdStep]=stairs(duty,pkg_std);

set (0, "defaultaxesfontname", "Helvetica")
set (0, "defaultaxesfontsize", 12) 
set (0, "defaultaxeslinewidth",2)
figure
plot(duty,p_mean,'k','linewidth',2); hold on;
plot(duty,p_mean+p_std,'m','linewidth',2);
plot(duty,p_mean-p_std,'m','linewidth',2);
ylabel('Power [W]');
xlabel('Duty = {T_{work}} / {(T_{work} + T_{sleep})}');
L = legend('Mean (200Sa) DAQ','Mean +/- Std.');
legend('location','northwest')

figure
dutyFine=[0.001:0.001:0.5];
pStdFine=interp1(duty,p_std,dutyFine,'spline',"extrap");
pMeanFine=interp1(duty,p_mean,dutyFine,'spline',"extrap");
%patch([duty,fliplr(duty)],[(p_mean-p_std),flipud(p_mean+p_std)],[1 0.6 1]); hold on;
topStd=(pMeanFine+pStdFine);
bottomStd=(pMeanFine-pStdFine);
patch([dutyFine,fliplr(dutyFine)],[bottomStd,fliplr(topStd)],[1 0.7 1],'edgecolor',[1 0.7 1]); hold on;
plot(dutyFine,pMeanFine,'r','linewidth',3);
legend('std','mean','location','northwest')
ylabel('Power [W]');
xlabel('$Duty = \frac{T_{work}}{T_{work} + T_{sleep}}$','interpreter','tex');
title('Power vs. Duty Cycle for Open-Loop Signal Sleep Controller');
%dram_mean = arrayfun(@(i) mean(dram_pg(i:i+n-1)),1:n:length(dram_pg)-n+1)';
%dram_std = arrayfun(@(i) std(dram_pg(i:i+n-1)),1:n:length(dram_pg)-n+1)';
%[tDramStep,dramMeanStep]=stairs(duty,dram_mean);
%[tDramStep,dramStdStep]=stairs(duty,dram_std);

figure

y1=plot(tStep,pMeanStep,'m','linewidth',2,'displayname','mean(DAQ)'); hold on;
y2=plot(tStep,pStdStep+pMeanStep,'k','linewidth',2,'displayname','mean(DAQ)+/-std(DAQ)');
y3=plot(tStep,pMeanStep-pStdStep,'k','linewidth',2);

%y4=plot(tPkgStep,pkgMeanStep,'r','linewidth',2,'displayname','mean(PKG)');
%y5=plot(tPkgStep,pkgStdStep+pkgMeanStep,'b','linewidth',2,'displayname','mean(PKG)+/-std(PKG)');
%y6=plot(tPkgStep,pkgMeanStep-pkgStdStep,'b','linewidth',2);

%y7=plot(tDramStep,dramMeanStep+pkgMeanStep,'g','linewidth',2,'displayname','mean(PKG+DRAM)'); 
%plot(tDramStep,dramStdStep+dramMeanStep,'k','linewidth',1,'displayname','mean(DAQ)+/-std(DAQ)');
%plot(tDramStep,dramMeanStep-dramStdStep,'k','linewidth',1);

ylabel('Power [W]');
xlabel('Duty = {T_{work}} / {(T_{work} + T_{sleep})}','interpreter','tex');
legend
legend('location','northwest')
