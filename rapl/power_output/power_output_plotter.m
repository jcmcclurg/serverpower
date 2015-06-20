b=input('Type the name: ','s')
%b='355660772_1434773393';

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
avgNum = zeros(numSteps,1);
for i = 1:numSteps
	window = (a(:,1) >= (stepTimes(i)+riseTime)) & (a(:,1) < (stepTimes(i+1)-riseTime));
	windows(i) = a(window,2);
	avgNum(i) = sum(window);
	%plot(a(window,1),a(window,2))
end

[v,i] = sort(stepValues);
boxplot(windows(i(1:10:numSteps)));
set(gca(),'xtick', 1:100:numSteps, 'xticklabel', v(1:100:numSteps) );
title('CPU duty cycle vs average power')
ylabel('Power (W) ')
xlabel('Duty cycle (percent)')
%print([b '.svg'],'-S1280x1024')
mean(avgNum)
