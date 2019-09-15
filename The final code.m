%% Open the text file from the generating property of matlab
filename = 'C:\record.txt';
delimiter = ';';
formatSpec = '%s%s%s%[^\n\r]';
fileID = fopen(filename,'r');
dataArray = textscan(fileID, formatSpec, 'Delimiter', delimiter,  'ReturnOnError', false);
fclose(fileID);
raw = repmat({''},length(dataArray{1}),length(dataArray)-1);
for col=1:length(dataArray)-1
    raw(1:length(dataArray{col}),col) = dataArray{col};
end
numericData = NaN(size(dataArray{1},1),size(dataArray,2));
for col=[1,2,3]
    rawData = dataArray{col};
    for row=1:size(rawData, 1);
        regexstr = '(?<prefix>.*?)(?<numbers>([-]*(\d+[\,]*)+[\.]{0,1}\d*[eEdD]{0,1}[-+]*\d*[i]{0,1})|([-]*(\d+[\,]*)*[\.]{1,1}\d+[eEdD]{0,1}[-+]*\d*[i]{0,1}))(?<suffix>.*)';
        try
            result = regexp(rawData{row}, regexstr, 'names');
            numbers = result.numbers;
            
            invalidThousandsSeparator = false;
            if any(numbers==',');
                thousandsRegExp = '^\d+?(\,\d{3})*\.{0,1}\d*$';
                if isempty(regexp(thousandsRegExp, ',', 'once'));
                    numbers = NaN;
                    invalidThousandsSeparator = true;
                end
            end
            
            if ~invalidThousandsSeparator;
                numbers = textscan(strrep(numbers, ',', ''), '%f');
                numericData(row, col) = numbers{1};
                raw{row, col} = numbers{1};
            end
        catch me
        end
    end
end



R = cellfun(@(x) ~isnumeric(x) && ~islogical(x),raw);
raw(R) = {NaN}; 

X = cell2mat(raw(:, 1));
Y = cell2mat(raw(:, 2));
Z = cell2mat(raw(:, 3));


clearvars filename delimiter formatSpec fileID dataArray ans raw col numericData rawData row regexstr result numbers invalidThousandsSeparator thousandsRegExp me R;

%% filter phase 
%Savitzky-Golay has been implemented of 3 and a frame length of 11.
x_smoothed= sgolayfilt(X,3,11);
y_smoothed=sgolayfilt(Y,3,11);
z_smoothed=sgolayfilt(Z,3,11);

%insert the smoothed data into a matrix 
Hd=[x_smoothed,y_smoothed,z_smoothed];
    L2=[];
 % calculating  the second euclidean distance to determine the start point of measurements  
for i=1:1:length(x_smoothed)
    L2(i) = norm(Hd(i,:));
end
L2_diff = diff(L2);

%calculating the mean for three axises 
mean_x_smoothed= mean(x_smoothed);
for i=1:1:length(x_smoothed)
    x_smoothed(i)=x_smoothed(i)-mean_x_smoothed;
end

mean_y_smoothed= mean(y_smoothed);
for i=1:1:length(y_smoothed)
    y_smoothed(i)=y_smoothed(i)-mean_y_smoothed;
end

mean_z_smoothed= mean(z_smoothed);
for i=1:1:length(z_smoothed)
    z_smoothed(i)=z_smoothed(i)-mean_z_smoothed;
end

% ploting the processed data 
subplot(4,1,1)
    hold on 
    % ploting x & x_smoothed 
    plot(1:length(X),X,'g')
    plot(1:length(X),x_smoothed,'b') 
    xlabel('X');
    grid
    hold off
subplot(4,1,2)    
    hold on 
        % ploting y & y_smoothed 
    plot(1:length(Y),Y,'g')
    plot(1:length(Y),y_smoothed,'b') 
     xlabel('Y');
     grid
    hold off 
    
    subplot(4,1,3)
 hold on 
     % ploting z & z_smoothed 
    plot(1:length(Z),Z,'g')
    plot(1:length(Z),z_smoothed,'b') 
    xlabel('Z');
    grid
    hold off
        % ploting L2
subplot(4,1,4)
plot(1:length(L2_diff),L2_diff,'g')


%% finding refernce signls to compare to 

x_vorne= zeros(1,30);

% cutting out the ith samples from the soomthed signal which respresnts the
% forwards  refernce signal for x axis
for i=1:1:30
    x_vorne(i)=x_smoothed(i+270);
end 
   
% cutting out the ith samples from the soomthed signal which respresnts the
% backwards refernce signal for x axis
x_rueckwaerts= zeros(1,30);

for i=1:1:30
    x_rueckwaerts(i)=x_smoothed(i+334);
end
    
%calculating the correlation between refernce signals and the original
%signls 
[r_r,lags_r]=xcorr(x_smoothed,x_rueckwaerts);
[r_v,lags_v]=xcorr(x_smoothed,x_vorne);

%ploting the smoothed original signal and the refernce signals and the
%correlation results

figure
subplot(4,1,1)
hold on
plot(x_smoothed)
legend('orignial')
subplot(4,1,2)

plot(lags_v,r_v)
axis([0 700 -inf inf] )
legend('x_vorne')
subplot(4,1,3)
plot(lags_r,r_r)
axis([0 700 -inf inf] )
legend('x_rückwärts')

subplot(4,1,4)
r_all= r_v-r_r;
plot(lags_v,r_all)
axis([0 700 -inf inf] )


% cutting out the ith samples from the soomthed signal which respresnts the
% right refernce signal for y axis
y_rechts= zeros(1,40);

for i=1:1:40
    y_rechts(i)=y_smoothed(i+560);
end


% cutting out the ith samples from the soomthed signal which respresnts the
%  left refernce signal fory axis    
y_links= zeros(1,35);

for i=1:1:35
    y_links(i) =y_smoothed(i+625);
end


% finding the peaks mean determine the movement
peaks=findpeaks(r_all)
[r_rechts,lags_rechts]=xcorr(y_smoothed,y_rechts);
[r_links,lags_links]=xcorr(y_smoothed,y_links);



%ploting the smoothed original signal and the refernce signals and the
%correlation results
figure
subplot(3,1,1)
hold on
plot(y_smoothed)
legend('orignial')
subplot(3,1,2)

plot(lags_links,r_links)
axis([0 700 -inf inf] )
legend('y_links')
subplot(3,1,3)
plot(lags_rechts,r_rechts)
axis([0 700 -inf inf] )
legend('y_rechts')






