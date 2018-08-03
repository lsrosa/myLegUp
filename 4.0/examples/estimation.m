arg_list = argv();

%create the name for the file with the estimations
folder = strrep(arg_list{1}, 'top.map.rpt', '');
outname = strcat(folder, 'hardware_estimation.mat');

%get resources from quartus map
measures = {'Cycles', 'ALMs', 'ALUTs', 'REGs', 'MEMBITs', 'DSPs'};
values = zeros(1, numel(measures));

%----------------------------------------------------------------------

%get cycles from moedelsim
modelsimreport = strcat(folder, 'transcript');
fid = fopen(modelsimreport, 'r')

cnt=0;
line = fgets(fid);
while(numel(strfind(line, "** Note: $finish")) == 0)
 cnt = cnt+1;
 line = fgets(fid);

 %exit if the file ends
 if line == -1
   %TODO exit an error maybe?
   break
 end
end

cnt = cnt-1;

%reset the pointer
frewind(fid);

fskipl (fid, cnt);
line = fgets(fid);
values(1) = strread(line, '%d')(3); %this one reads empty spaces as 0

fclose(fid);


%----------------------------------------------------------------------
%find the first os ALM resources
comment = 'getting resources'

%read the quartus report file
fid = fopen(arg_list{1}, 'r');
cnt=0;
line = fgets(fid);

while(numel(strfind(line, "Estimate of Logic utilization (ALMs needed)")) == 0)
 cnt = cnt+1;
 line = fgets(fid);

 %exit if the file ends
 if line == -1
   %TODO exit an error maybe?
   break
 end
end

%reset the pointer
frewind(fid);

%how many lines to skip between each measure
skips = [cnt, 1, 6, 3, 1];

for i=1:numel(skips)
  fskipl (fid, skips(i));
  line = fgets(fid);
  values(i+1) = strread(line, '%d')(end-1); %this one reads empty spaces as 0
end

values

fclose(fid);

%write a smaller and readable report
outname = outname
save(outname, 'measures', 'values')
