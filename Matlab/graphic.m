if exist('s1', 'var')  %判断上一次打开的端口有没有关闭
    clear s1;
end

%新建串口对象
s1=serialport('COM9',115200); %设置串口波特率
s1.InputBufferSize = 8000;        %输入缓冲区长度8000字节
s1.Timeout=4;
index=51;
buffer=zeros(51);
while 1
    buffer=circshift(buffer,-1);
    str=readline(s1);
    buffer(index) = str2double(str);%read(s1,1,"UINT16");
    plot(buffer);                      %数据点之间连线
    drawnow
end
