samplesPerStep = 100;
numSteps = 100;

length = 10000;
blockLength = 1000;
sampleRate = (length/blockLength);

durationPerStep = samplesPerStep/sampleRate;
t = (0:(numSteps-1))*durationPerStep;

minVal = 1200000;
maxVal = 2201000;

r = linspace(minVal,maxVal,numSteps);
z = zeros(numSteps,2);
z(:,1) = t;
z(1:2:end,2) = r(1:2:end);
z(2:2:end,2) = fliplr(r(2:2:end));

dlmwrite(sprintf('ramp_%d_%d.csv',minVal,maxVal),z,',')
dlmwrite(sprintf('ramp_%d_%d_short.csv',minVal,maxVal),[[0 3 6 9]; [0.1 0.2 1 0]*(maxVal - minVal) + minVal]',',')
