% Limpar leituras e gráficos anteriores
clear;
close all;

% Configuração da porta serial
portaSerial = '/dev/ttyUSB0'; % Substitua pela sua porta serial (exemplo: COM3 no Windows)
taxaBits = 115200;            % Taxa de bits definida no código Arduino

% Abre a conexão serial
s = serialport(portaSerial, taxaBits);

% Inicializa os gráficos
figure(1);
h1 = animatedline('Color', 'b', 'DisplayName', 'Referência de Velocidade');
h2 = animatedline('Color', 'r', 'DisplayName', 'Velocidade Atual');
legend;
title('Controle de Velocidade do Motor');
xlabel('Tempo (s)');
ylabel('Velocidade (RPM)');
grid on;

% Tempo de execução
tempoInicial = datetime('now');
duration = seconds(40); % Tempo de execução do gráfico (ajuste conforme necessário)

% Inicializa arrays para armazenar dados de tempo e leitura
tempos = [];
referencias = [];
velocidades = [];

% Leitura e plotagem em tempo real
while datetime('now') - tempoInicial < duration
    if s.NumBytesAvailable > 0
        % Lê os valores do Arduino
        data = readline(s); % Lê uma linha da porta serial
        valores = str2double(split(data, ' ')); % Divide pelos separadores de espaço

        % Verifica se os dados são válidos
        if numel(valores) == 2
            referencia = valores(1);      % Referência de velocidade (RPM)
            velocidade = valores(2);      % Velocidade atual do motor (RPM)

            % Obtém o tempo atual e adiciona o ponto ao gráfico
            tempoAtual = datetime('now') - tempoInicial;
            tempoSegundos = seconds(tempoAtual);

            % Adiciona dados aos arrays
            tempos = [tempos; tempoSegundos];
            referencias = [referencias; referencia];
            velocidades = [velocidades; velocidade];

            % Adiciona pontos aos gráficos
            addpoints(h1, tempoSegundos, referencia);
            addpoints(h2, tempoSegundos, velocidade);
            drawnow;
        end
    end
end

% Fecha a conexão serial
clear s;

% Salva os dados em um arquivo
dadosSalvos = [tempos, referencias, velocidades];
writematrix(dadosSalvos, 'controle_velocidade_motor.txt', 'Delimiter', '\t');

% Gera o gráfico final com os dados armazenados
figure(2);
plot(tempos, referencias, 'b', 'DisplayName', 'Referência de Velocidade');
hold on;
plot(tempos, velocidades, 'r', 'DisplayName', 'Velocidade Atual');
title('Dados armazenados do controle de velocidade do motor');
xlabel('Tempo (s)');
ylabel('Velocidade (RPM)');
legend;
grid on;