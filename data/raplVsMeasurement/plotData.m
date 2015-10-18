% plotData.m
clear all; clc; close all;
source("~/Documents/research/serverpower/data/raplVsMeasurement/loadData.m");

figure
%plot(t_pg,pkg_pg,'->r','linewidth',1,'displayname','RAPL PKG'); hold on
%plot(t_pg,dram_pg+pkg_pg,'-<b','linewidth',1,'displayname','RAPL PKG+DRAM');
%plot(t_rm,p_rm,'-^k','linewidth',1,'displayname','DAQ');

n = 100; % sample averaging window
% average pkg over window n
p_mean = arrayfun(@(i) mean(p_rm(i:i+n-1)),1:n:length(p_rm)-n+1)';
p_std = arrayfun(@(i) std(p_rm(i:i+n-1)),1:n:length(p_rm)-n+1)';
t_new = arrayfun(@(i) t_rm(i),1:n:length(p_rm)-n+1)';
[tStep,pMeanStep]=stairs(t_new,p_mean);
[tStep,pStdStep]=stairs(t_new,p_std);

pkg_mean = arrayfun(@(i) mean(pkg_pg(i:i+n-1)),1:n:length(pkg_pg)-n+1)';
pkg_std = arrayfun(@(i) std(pkg_pg(i:i+n-1)),1:n:length(pkg_pg)-n+1)';
t_new_pg = arrayfun(@(i) t_pg(i),1:n:length(pkg_pg)-n+1)';
[tPkgStep,pkgMeanStep]=stairs(t_new_pg,pkg_mean);
[tPkgStep,pkgStdStep]=stairs(t_new_pg,pkg_std);

dram_mean = arrayfun(@(i) mean(dram_pg(i:i+n-1)),1:n:length(dram_pg)-n+1)';
dram_std = arrayfun(@(i) std(dram_pg(i:i+n-1)),1:n:length(dram_pg)-n+1)';
[tDramStep,dramMeanStep]=stairs(t_new_pg,dram_mean);
[tDramStep,dramStdStep]=stairs(t_new_pg,dram_std);

y1=plot(tStep,pMeanStep,'m','linewidth',2,'displayname','mean(DAQ)'); hold on;
y2=plot(tStep,pStdStep+pMeanStep,'k','linewidth',2,'displayname','mean(DAQ)+/-std(DAQ)');
y3=plot(tStep,pMeanStep-pStdStep,'k','linewidth',2);

y4=plot(tPkgStep,pkgMeanStep,'r','linewidth',2,'displayname','mean(PKG)');
y5=plot(tPkgStep,pkgStdStep+pkgMeanStep,'b','linewidth',2,'displayname','mean(PKG)+/-std(PKG)');
y6=plot(tPkgStep,pkgMeanStep-pkgStdStep,'b','linewidth',2);

y7=plot(tDramStep,dramMeanStep+pkgMeanStep,'g','linewidth',2,'displayname','mean(PKG+DRAM)'); 
%plot(tDramStep,dramStdStep+dramMeanStep,'k','linewidth',1,'displayname','mean(DAQ)+/-std(DAQ)');
%plot(tDramStep,dramMeanStep-dramStdStep,'k','linewidth',1);


ylabel('Power [W]');
xlabel('Time [sec]');
legend
legend('location','northwest')
