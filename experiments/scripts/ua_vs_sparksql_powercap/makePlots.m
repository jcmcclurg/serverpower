% The stage inputs are already in row format
stageInputs = dlmread('stageInputs.txt',' ',1,0);
stageInputs = sortrows(stageInputs,1);

% "-1" means no power cap, which means 100% available power
stageInputs(stageInputs(:,2) == -1,2) = 100;


% The power results are transposed
powerResults = dlmread('powerResultSummary.txt','\t',0,1)';
powerResults = sortrows(powerResults,1);


% The stage results are transposed
stageResults = dlmread('stageResultSummary.txt','\t',0,1)';
stageResults = sortrows(stageResults,1);

topindex=7; toplabel = 'update latency';
bottomindex=6; bottomlabel = 'select latency';
selectivityIndex=6;
powerCapIndex=2;
isSQLIndex=3;
precondition = stageInputs(:,selectivityIndex) != 1.0;

subplot(2,2,1)
condition = stageInputs(:,isSQLIndex) == 0 & precondition;
plotMaker(stageInputs(condition,:),stageResults(condition,:),...
  powerCapIndex,'power cap',...
  topindex,toplabel,...
  selectivityIndex,'sz',...
  'UABitmaps');

subplot(2,2,3)
condition = stageInputs(:,isSQLIndex) == 0 & precondition;
plotMaker(stageInputs(condition,:),stageResults(condition,:),...
  powerCapIndex,'power cap',...
  bottomindex,bottomlabel,...
  selectivityIndex,'sz',...
  'UABitmaps');


subplot(2,2,2)
condition = stageInputs(:,isSQLIndex) == 1 & precondition;
plotMaker(stageInputs(condition,:),stageResults(condition,:),...
  powerCapIndex,'power cap',...
  topindex,toplabel,...
  selectivityIndex,'sz',...
  'SparkSQL');

subplot(2,2,4)
condition = stageInputs(:,isSQLIndex) == 1 & precondition;
plotMaker(stageInputs(condition,:),stageResults(condition,:),...
  powerCapIndex,'power cap',...
  bottomindex,bottomlabel,...
  selectivityIndex,'sz',...
  'SparkSQL');