if exist('s1', 'var')  %判断上一次打开的端口有没有关闭
    clear s1;
end

%新建串口对象
s1=serialport('COM9',115200); %设置串口波特率
s1.InputBufferSize = 8000;        %输入缓冲区长度8000字节
s1.Timeout=4;

%ADC采样
index1=201;
buffer1=zeros(1,201);
subplot(2,1,1);
plot1=plot(buffer1);
axis([0 inf 0 4096]);

%心率
index2=51;
buffer2=zeros(1,51);
subplot(2,1,2);
plot2=plot(buffer2);
axis([0 inf 50 150]);
yticks(50:10:150);

while 1
    str=readline(s1);
    if not (isempty(str))
        buffer1=circshift(buffer1,-1);
        buffer2=circshift(buffer2,-1);

        datas=strsplit(str);
        data1=datas(1);
        data2=datas(2);

        buffer1(index1) = str2double(data1);
        buffer2(index2) = str2double(data2);

        set(plot1, 'YData', buffer1);
        set(plot2, 'YData', buffer2);

        drawnow 
    end
end
