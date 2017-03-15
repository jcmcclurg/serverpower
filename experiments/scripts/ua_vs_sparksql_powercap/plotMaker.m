function [plt] = plotMaker(stageInputs,stageResults,...
                          xindex,thexlabel,...
                          yindex,theylabel,...
                          subindex,thesublabel,...
                          thetitle)
  updateSizes = unique(stageInputs(:,subindex));
  legends={};
  for i = 1:length(updateSizes)
    us = updateSizes(i);
    subcondition = stageInputs(:,subindex) == us;
    x = stageInputs(subcondition,xindex);
    y = stageResults(subcondition,yindex);
    plt = plot(x,y);
    hold all
    legends{end+1} = sprintf('%s=%g',thesublabel,us);
  end
  hold off
  
  title(thetitle)
  xlabel(thexlabel)
  ylabel(theylabel)  
  legend(legends)
end