% plotData.m
clear all; clc; close all;
source("~/Documents/research/serverpower/data/raplVsMeasurement/loadData.m");

figure
plot(t_pg,pkg_pg,'->r','linewidth',2,'displayname','RAPL PKG'); hold on
plot(t_pg,dram_pg+pkg_pg,'-<b','linewidth',2,'displayname','RAPL PKG+DRAM');
plot(t_rm,p_rm,'-^k','linewidth',2,'displayname','DAQ');
ylabel('Power [W]');
xlabel('Time [sec]');
legend
legend boxoff
legend('location','northwest')
