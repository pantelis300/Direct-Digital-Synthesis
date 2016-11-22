fileID = fopen('test', 'r');

sine_wave = fscanf(fileID, '%d');
t = linspace(0, 8*pi, length(sine_wave));

figure;
hold on
plot(t, sine_wave)
hold off
