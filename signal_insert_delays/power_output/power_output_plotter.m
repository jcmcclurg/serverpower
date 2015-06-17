b="737849824_1434205397";
a = dlmread([b ".output"]," ");
a(:,1) = a(:,1) - a(1,1);
badTimes = a(:,1) < 0;
a(badTimes,:) = [];

plot(a(:,1),a(:,2))
