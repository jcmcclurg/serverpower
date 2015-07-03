%read_pwr_data.m
close all;
clear all;
clc;

% open file and get current data
fid = fopen('scope_26_1.csv');
data_iac = textscan(fid,'%f %f','Delimiter',',','HeaderLines',2);
fclose(fid);
t_iac = data_iac{1}; % time (seconds)
iac = data_iac{2}/2.0; % voltage_measurement/gain = current (Amps)

% open file and get voltage data
fid = fopen('scope_26_2.csv');
data_vac = textscan(fid,'%f %f','Delimiter',',','HeaderLines',2);
fclose(fid);
t_vac = data_vac{1}; % time (seconds)
vac = data_vac{2}; % voltage_measurement
data_scope = [t_iac iac t_vac vac];
save data_scope.mat data_scope

%source('~joe/serverpower/rapl/data/to_rms.m')
