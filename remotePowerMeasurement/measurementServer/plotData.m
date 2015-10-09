if ~exist('data')
  load('data.mat')
end

voltageScalingFactor = 124.0/6.55;

rackResistor = 0.02;
rackGain = 10;
rackScalingFactor = 1.0/(rackResistor*rackGain);

serverResistor = 0.02;
serverGain = 50;
serverScalingFactor = 1.0/(serverResistor*serverGain);

rng = 1:1000;
subplot(3,1,1)
plot(time(rng),voltageScalingFactor*v0(rng))

subplot(3,1,2)
plot(time(rng),rackScalingFactor*v1(rng))
hold all
plot(time(rng),serverScalingFactor*sum(data(rng,4:8),2))
hold off

subplot(3,1,3)
plot(time(rng),serverScalingFactor*v2(rng))
hold all
plot(time(rng),serverScalingFactor*v3(rng))
plot(time(rng),serverScalingFactor*v4(rng))
plot(time(rng),serverScalingFactor*v5(rng))
plot(time(rng),serverScalingFactor*v6(rng))
hold off