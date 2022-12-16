clc
close all
clear all

%% pre allocation

recording_frequency = 1000; % hz
time_to_record = 10; % seconds

pre_allocated_space = floor(time_to_record*recording_frequency); %% 1 loop collects the data for 20 msec

FSR_data = zeros(pre_allocated_space,8);

%% Connect to Arduino

serial_list = serialportlist("available");

[index,tf] = listdlg('PromptString','Choose the Serial Port to use', 'Selectionmode', 'single','ListSize',[150,50], 'ListString' ,serial_list);

port = serial_list{index};
s = serialport(port, 115200);

%% plot data
figure
a1 = animatedline('Color', '#ffdaaa');
a2 = animatedline('Color', '#f3bf99');
a3 = animatedline('Color', '#e6a589');
a4 = animatedline('Color', '#d68b79');
a5 = animatedline('Color', '#c5726b');
a6 = animatedline('Color', '#b15b5e');
a7 = animatedline('Color', '#994852');
a8 = animatedline('Color', '#66454f');
axis([0,cast(pre_allocated_space, 'int64'), 0, 255])

x = linspace(1,pre_allocated_space, pre_allocated_space);

for i=1:cast(pre_allocated_space,'int64')
    data = read(s, 8, "uint8");
    FSR_data(i,:) = data(1:8);
    addpoints(a1,x(i),data(1));
    addpoints(a2,x(i),data(2));
    addpoints(a3,x(i),data(3));
    addpoints(a4,x(i),data(4));
    addpoints(a5,x(i),data(5));
    addpoints(a6,x(i),data(6));
    addpoints(a7,x(i),data(7));
    addpoints(a8,x(i),data(8));
end

save('FSR_data.mat',"FSR_data")

s.write("disconnect", "char")