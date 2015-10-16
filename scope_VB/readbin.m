fileID = fopen("ScopeDataieeeread.bin");
block = fread(fileID);
fclose(fileID);
dataPoints = 0;
if (block(1)==35 && block(2)==56) % check header begins with "#8"
	for i=0:7
		dataPoints += (block(3+i)-48)*10**(7-i);
	end
	bitsPerPoint = round(length(block(11:end-1))/dataPoints*8);
	%precision = sprintf('*ubit%d', bitsPerPoint);
	%fileID = fopen("ScopeData.bin",precision);
else
	fprintf('header specifier not found.\n');
end

