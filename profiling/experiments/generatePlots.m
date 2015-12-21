if ~exist('data')
	pkg load statistics;

	experiment = 'stress';
	date = '1450289756.980881700';

	cd(experiment)
	cd(date)

	data = dlmread('powerlog.log',' ');

	measuredPowers = [data(:,1) data(:,4:7)];
	serverSetpoints = {};

	for i = 1:4
		cd(sprintf('server%d',i));
		data = dlmread(sprintf('%s.testlog',date),',',3,0);
		s = struct('setpoints',data(1:(end-12),:),'measuredPower',{ {} }, 'stats', { {} });
		serverSetpoints{i} = s;

		% go through every setpoint except the last one because the test ends at the last one
		for j = 1:(size(serverSetpoints{i}.setpoints,1) - 1)
			t0 = serverSetpoints{i}.setpoints(j,1);
			p = measuredPowers( ...
			(measuredPowers(:,1) >= serverSetpoints{i}.setpoints(j,1)) & ...
			(measuredPowers(:,1) < serverSetpoints{i}.setpoints(j+1,1)) ...
			, [1 i+1]);

			% Record the measured power
			serverSetpoints{i}.measuredPower{j} = p;

			% Calculate settling time, defined by the interval +/- 2*stddev
			samps = p(20:end,2);
			finalValue = mean(samps);
			stddev = std(samps);
			k = find((p(1:19,2) > 2*stddev + finalValue) | (p(1:19,2) < 2*stddev - finalValue),1,'last');
			if length(k) == 0
				k = 0;
			end
			settlingTime = p(k+1,1)-t0;

			% Calculate 95 percent confidence interval for mean.
			samps = p(k+1:end,2);
			finalValue = mean(samps);
			[h, pval, ci, stats] = ttest(samps,finalValue);

			% Divide the samples into quartiles
			s = sort(samps);
			minSegmentLen = floor(length(s)/4.0);
			numRemainders = mod(length(s),4);
			segmentLengths = ones(4,1)*minSegmentLen;
			segmentLengths(1:numRemainders) = segmentLengths(1:numRemainders) + 1;
			endpoints = cumsum(segmentLengths(randperm(4)));

			q1 = s(1:endpoints(1));
			q2 = s(endpoints(1)+1:endpoints(2));
			q3 = s(endpoints(2)+1:endpoints(3));
			q4 = s(endpoints(3)+1:endpoints(4));

			serverSetpoints{i}.stats{j} = struct('settlingTime', settlingTime, 'stddev', stddev, 'mean', finalValue, 'ci', ci, 'pval', pval, 'nsamps', length(samps), 'iqr', [q2(1) q3(end)], 'range', [q1(1) q4(end)] );
	%		fprintf(2,'Server %d, setpoint %d, length %d, 95%% CI: +/-%d\n',i,j,length(p(k+1:end,2)),max(ci))
	%		if j == 5
	%			fprintf(2,'s: ');
	%			fprintf(2,'%0.2f ',s);
	%			fprintf(2,'\nq1: ');
	%			fprintf(2,'%0.2f ',q1);
	%			fprintf(2,'\nq2: ');
	%			fprintf(2,'%0.2f ',q2);
	%			fprintf(2,'\nq3: ');
	%			fprintf(2,'%0.2f ',q3);
	%			fprintf(2,'\nq4: ');
	%			fprintf(2,'%0.2f ',q4);
	%			fprintf(2,'\n');
	%		end
		end
		cd('..')
	end
	cd('../..')
end

j = 5;
i = 4;
%for i = 1:4
%	data = {}
%	for j = 1:(size(serverSetpoints{i}.setpoints,1) - 1)
%		t0 = serverSetpoints{i}.setpoints(j,1);
%		p = serverSetpoints{i}.measuredPower{j};
%		k = size(p,1) - serverSetpoints{i}.stats{j}.nsamps+1;
%		data{j} = p(k:end,2);
%	end
%	boxplot(data);
%end
%axis([0 t1-t0 0 120])

i = 1;
for i = 1:4
	setpoints = zeros(size(serverSetpoints{i}.setpoints,1) - 1,1);
	means = zeros(size(serverSetpoints{i}.setpoints,1) - 1,1);
	uerr = zeros(size(serverSetpoints{i}.setpoints,1) - 1,1);
	lerr = zeros(size(serverSetpoints{i}.setpoints,1) - 1,1);
	for j = 1:(size(serverSetpoints{i}.setpoints,1) - 1)
		setpoints(j) = serverSetpoints{i}.setpoints(j,2);
		means(j) = serverSetpoints{i}.stats{j}.mean;
		%uerr(j) = serverSetpoints{i}.stats{j}.iqr(2) - means(j);
		%lerr(j) = means(j) - serverSetpoints{i}.stats{j}.iqr(1);
		uerr(j) = serverSetpoints{i}.stats{j}.range(2) - means(j);
		lerr(j) = means(j) - serverSetpoints{i}.stats{j}.range(1);
		%uerr(j) = serverSetpoints{i}.stats{j}.ci(2);
		%lerr(j) = -serverSetpoints{i}.stats{j}.ci(1);
	end
	[setpoints,order] = sort(setpoints);
	h = errorbar(setpoints,means(order),lerr(order),uerr(order),'~*');
	c = get(h,'Children');
	set(c(1),'LineWidth',3);
	set(c(2),'LineWidth',0.5);
	hold all;
end
hold off;
