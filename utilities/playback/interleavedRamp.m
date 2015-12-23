samplesPerStep = 1000;
numSteps = 100;

length = 10000;
blockLength = 1000;
sampleRate = (length/blockLength);

durationPerStep = samplesPerStep/sampleRate;
t = (0:(numSteps-1))*durationPerStep;



names = {'cpufreq','powerclamp','stress','signal_insert_delays','hypervisor','rapl'}
minVals = [1200000 0  0 0 1    21];
maxVals = [2201000 50 1 1 1200 36];
ints    = [1       0  0 0 1    0 ];

for i = 1:size(maxVals,2)
	minVal = minVals(i);
	maxVal = maxVals(i);
	r = linspace(minVal,maxVal,numSteps);
	z = zeros(numSteps,2);
	z(:,1) = t;
	z(1:2:end,2) = r(1:2:end);

	if ints(i)
		z(2:2:end,2) = round(fliplr(r(2:2:end)));
		zs = [[0 3 6 9]; round([0.1 0.2 1 0]*(maxVal - minVal) + minVal)]';
	else
		z(2:2:end,2) = fliplr(r(2:2:end));
		zs = [[0 3 6 9]; [0.1 0.2 1 0]*(maxVal - minVal) + minVal]';
	end

	dlmwrite(sprintf('../../power_modulating_interfaces/%s/interleavedRamp_%d_%d.csv',names{i},minVal,maxVal),z,',')
	dlmwrite(sprintf('../../power_modulating_interfaces/%s/interleavedRamp_%d_%d_short.csv',names{i},minVal,maxVal),zs,',')
end
