if exist('s1', 'var')  %判断上一次打开的端口有没有关闭
    clear s1;
end

%新建串口对象
s1=serialport('COM9',115200);     %设置串口波特率
s1.InputBufferSize = 8000;        %输入缓冲区长度8000字节
s1.Timeout=4;

%ADC采样
index1=201;               %adc采样bufffer的大小
buffer1=zeros(1,index1);  %定义adc buffer

%配置图标的显示方式
subplot(2,1,1);            
plot1=plot(buffer1);
axis([0 inf 0 4096]);

%心率
index2=51;               %心率数据bufffer的大小
buffer2=zeros(1,index2); %定义心率buffer

%配置图标的显示方式
subplot(2,1,2);
plot2=plot(buffer2);
axis([0 inf 50 150]);
yticks(50:10:150);

while 1
    %从串口读取一行数据
    str=readline(s1);
    if not (isempty(str))    %若成功读取到数据
        datas=strsplit(str); %以空格分割字符串

        data1=datas(1);                     %第一部分为adc数据
        buffer1=circshift(buffer1,-1);      %将buffer左移一位
        buffer1(index1) = str2double(data1);%将buffer的最后一位设置为新的值
        set(plot1, 'YData', buffer1);       %设置折线图表新值

        data2=datas(2);                     %第二部分为心率数据
        buffer2=circshift(buffer2,-1);      %将buffer左移一位
        buffer2(index2) = str2double(data2);%将buffer的最后一位设置为新的值
        set(plot2, 'YData', buffer2);       %设置折线图表新值

        drawnow 
    end
end
