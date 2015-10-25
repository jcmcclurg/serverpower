% plotData.m
clear all; clc; close all;
source("~/Documents/research/serverpower/data/loop1server/loadData.m");
set(0,'defaultaxesfontname','Helvetica');
set(0,'defaultaxesfontsize',14);
set(0,'defaultaxeslinewidth',2);
figure
yr=plot(t_pg,pkg_pg,'-or','linewidth',2,'displayname','RAPL PKG'); hold on
%plot(t_pg,dram_pg+pkg_pg,'-<b','linewidth',1,'displayname','RAPL PKG+DRAM');
yd=plot(t_rm,p_rm,'->k','linewidth',2,'displayname','DAQ');
%plot(t_fmd1,sp_fmd1,'.m','linewidth',1,'displayname','Power Setpoint');
ys=plot([36468.4 36468.4],[-5,55],'--b','displayname','start of step input','linewidth',2');
slope1=plot([36468.57 36468.745],[37,70],'linewidth',3,'--m');
slope2=plot([36468.545 36468.805],[5,39],'linewidth',3,'--m');
l1=plot([36468.4 36468.57],[35 35],'linewidth',3,'m','marker','+','markersize',10);
l2=plot([36468.4 36468.54],[2 2],'linewidth',3,'m','marker','+','markersize',10);
grid on;
R1=(p_rm(33)-p_rm(32))/(t_rm(33)-t_rm(32))
R2=(pkg_pg(32)-pkg_pg(31))/(t_pg(32)-t_pg(31))
L1=36468.57-36468.4
L2=36468.54-36468.4
alpha1=L1*R1
alpha2=L2*R2
K1=0.9/alpha1
K2=0.9/alpha2
Ti1=3*L1
Ti2=3*L2
r1str=sprintf("R_{DAQ} = %.2fW/s",R1)
r1txt=text(t_rm(33),p_rm(33)-5,r1str,'interpreter','tex','fontsize',14,'fontname','Helvetica')
r2str=sprintf("R_{RAPL} = %.2fW/s",R2)
r2txt=text(t_pg(32),pkg_pg(32)-5,r2str,'interpreter','tex','fontsize',14,'fontname','Helvetica')
l1str=sprintf("L_{DAQ} = %.2fs",L1)
l2str=sprintf("L_{RAPL} = %.2fs",L2)
l1txt=text(36468.41,30,l1str,'interpreter','tex','fontsize',14,'fontname','Helvetica');
l2txt=text(36468.41,-5,l2str,'interpreter','tex','fontsize',14,'fontname','Helvetica');
%str=sprintf("Ziegler-Nichols Tuning Rules\nfor PI Parameters\n \tK\tT_i\n \t1/RL\t3L");
ylabel('Power [W]');
xlabel('Time [sec]');
legend
legend('location','northwest')
legend boxoff
title('Apparent DeadTime, L, and Steepest Slope, R, of Open-Loop Step-Response');
box off
%{
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
y5=plot(tPkgStep,pkgStdStep+pkgMeanStep,'g','linewidth',2,'displayname','mean(PKG)+/-std(PKG)');
y6=plot(tPkgStep,pkgMeanStep-pkgStdStep,'g','linewidth',2);
y4=plot(tPkgStep,pkgMeanStep,'m','linewidth',2,'displayname','mean(PKG)')
%y7=plot(tDramStep,dramMeanStep+pkgMeanStep,'g','linewidth',2,'displayname','mean(PKG+DRAM)'); 
%plot(tDramStep,dramStdStep+dramMeanStep,'k','linewidth',1,'displayname','mean(DAQ)+/-std(DAQ)');
%plot(tDramStep,dramMeanStep-dramStdStep,'k','linewidth',1);


ylabel('Power [W]');
xlabel('Time [sec]');
legend
legend('location','northwest')

