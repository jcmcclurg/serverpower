b=input('Type the name: ','s')
%b='244991366_1434682629';

[status, output] = system(['cat ./' b '.log | grep "Starting power gadget at " | cut -d \  -f 5']);
assert(status == 0);
startTime = str2double(output);

[status, output] = system(['cat ./' b '.log | grep "Step to " | cut -d \  -f 3']);
assert(status == 0);
output = strsplit(output);
stepValues = str2double(output);
stepValues = stepValues(!isnan(stepValues));

% Normalize the times to when the power gadget started.
[status, output] = system(['cat ./' b '.log | grep "Step to " | cut -d \  -f 5']);
assert(status == 0);
output = strsplit(output);
stepTimes = str2double(output) - startTime;

% Normalize the times to when the power gadget started.
a = dlmread(['./' b '.output'],' ');
a(:,1) = a(:,1) - a(1,1);
badTimes = a(:,1) < 0;
a(badTimes,:) = [];

riseTime = 0.1;

windows = {};
numSteps = (length(stepTimes)-1);
for i = 1:numSteps
	window = (a(:,1) >= (stepTimes(i)+riseTime)) & (a(:,1) < (stepTimes(i+1)-riseTime));
	windows(i) = a(window,2);
	%plot(a(window,1),a(window,2))
end

[v,i] = sort(stepValues);
plotI = i(1:10:numSteps);
plotV = v(1:10:numSteps);
boxplot(windows(plotI));
ticks = unique([1:5:length(plotI) length(plotI)]);

set(gca(),'xtick', ticks, 'xticklabel', 100*plotV(ticks) );
title('CPU duty cycle vs average power')
ylabel('Power (W) ')
xlabel('Duty cycle (percent)')
print([b '.png'],'-S1280x1024')
print('insert_delays.png','-S1280x1024')
