% plotData.m
clear all; clc; close all;
source("~/Documents/research/serverpower/data/loop1server/loadData.m");
set (0, "defaultaxesfontname", "Helvetica")
set (0, "defaultaxesfontsize", 12)
set (0, "defaulttextfontname", "Helvetica")
set (0, "defaulttextfontsize", 12) 
figure
plot(t_pg,pkg_pg,'.r','linewidth',1,'displayname','RAPL PKG'); hold on
%plot(t_pg,dram_pg+pkg_pg,'-<b','linewidth',1,'displayname','RAPL PKG+DRAM');
plot(t_rm,p_rm,'.k','linewidth',1,'displayname','DAQ');
plot(t_fmd1,sp_fmd1,'.m','linewidth',1,'displayname','Power Setpoint');
ylabel('Power [W]');
xlabel('Time [sec]');
legend
legend('location','northwest')

n = 20; % sample averaging window
% average pkg over window n
if !isnan(p_rm)
	p_mean = arrayfun(@(i) mean(p_rm(i:i+n-1)),1:n:length(p_rm)-n+1)';
	p_std = arrayfun(@(i) std(p_rm(i:i+n-1)),1:n:length(p_rm)-n+1)';
	t_new = arrayfun(@(i) t_rm(i),1:n:length(p_rm)-n+1)';
	[tStep,pMeanStep]=stairs(t_new,p_mean);
	[tStep,pStdStep]=stairs(t_new,p_std);
else
	p_mean = NaN;p_std = NaN;t_new = NaN;tStep =NaN;
	pMeanStep=NaN;tStep=NaN;pStdStep=NaN;
end

if !isnan(pkg_pg)
	pkg_mean = arrayfun(@(i) mean(pkg_pg(i:i+n-1)),1:n:length(pkg_pg)-n+1)';
	pkg_std = arrayfun(@(i) std(pkg_pg(i:i+n-1)),1:n:length(pkg_pg)-n+1)';
	t_new_pg = arrayfun(@(i) t_pg(i),1:n:length(pkg_pg)-n+1)';
	[tPkgStep,pkgMeanStep]=stairs(t_new_pg,pkg_mean);
	[tPkgStep,pkgStdStep]=stairs(t_new_pg,pkg_std);
else
	pkg_mean = NaN;pkg_std = NaN;t_new_pg = NaN;tPkgStep =NaN;
	pkgMeanStep=NaN;tPkgStep=NaN;pkgStdStep=NaN;
end
%{
dram_mean = arrayfun(@(i) mean(dram_pg(i:i+n-1)),1:n:length(dram_pg)-n+1)';
dram_std = arrayfun(@(i) std(dram_pg(i:i+n-1)),1:n:length(dram_pg)-n+1)';
[tDramStep,dramMeanStep]=stairs(t_new_pg,dram_mean);
[tDramStep,dramStdStep]=stairs(t_new_pg,dram_std);
%}
figure
plot(t_fmd1,sp_fmd1,'k','linewidth',2,'displayname','Power Setpoint'); hold on;
y2=plot(tStep,pStdStep+pMeanStep,'g','linewidth',2,'displayname','mean(DAQ)+/-std(DAQ)');
y3=plot(tStep,pMeanStep-pStdStep,'g','linewidth',2);
y1=plot(tStep,pMeanStep,'b','linewidth',2,'displayname','mean(DAQ)');
%y5=plot(tPkgStep,pkgStdStep+pkgMeanStep,'g','linewidth',2,'displayname','mean(PKG)+/-std(PKG)');
%y6=plot(tPkgStep,pkgMeanStep-pkgStdStep,'g','linewidth',2);
%y4=plot(tPkgStep,pkgMeanStep,'m','linewidth',2,'displayname','mean(PKG)')
%y7=plot(tDramStep,dramMeanStep+pkgMeanStep,'g','linewidth',2,'displayname','mean(PKG+DRAM)'); 
%plot(tDramStep,dramStdStep+dramMeanStep,'k','linewidth',1,'displayname','mean(DAQ)+/-std(DAQ)');
%plot(tDramStep,dramMeanStep-dramStdStep,'k','linewidth',1);


ylabel('Power [W]');
xlabel('Time [sec]');
legend
legend('location','northwest')

