## Copyright (C) 2015 U-Josiah-PC\Josiah
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*- 
## @deftypefn {Function File} {@var{retval} =} stressPlot (@var{input1}, @var{input2})
##
## @seealso{}
## @end deftypefn

## Author: U-Josiah-PC\Josiah <Josiah@Josiah-PC>
## Created: 2015-07-23

time='1438093353.275244806';

[status,output] = system(['cat hypervisor.' time '.output | grep HadoopKmeans | tail -n 1 | tr -s \  | cut -d \  -f 4,5']);
[inputSize duration]=strread(output,'%f %f');
fprintf(1,'Input size: %g GiB, Duration: %g min, Throughput: %g MiB/s\n', inputSize/(power(2,30)), duration/60, inputSize/(duration*power(2,20)))

[status,output] = system(['cat hypervisor.' time '.output | grep Using | cut -d \  -f 2']);
setpoint = dlmread(output(1:end-1),',');
actual = dlmread(['hypervisor.' time '.power'],' ');
actual(:,1) = actual(:,1) - actual(1,1) - 5;
actual(end-4:end,:) = [];

avtime = block_average(actual(:,1),4);
avactual = block_average(actual(:,2),4);

plot(actual(:,1),actual(:,2),'y',avtime,avactual,'c',[setpoint(:,1); actual(end,1)],[setpoint(:,2); setpoint(end,2)],'r')
title('Hypservisor power vs setpoint.')
legend('Actual (0.5s average)','Actual (2s average)','Setpoint')
xlabel('Time (s)')
ylabel('Power (W)')
axis([0, max(actual(:,1)), 4, 40]);
print('hypervisor.pdf')
