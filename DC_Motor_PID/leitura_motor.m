% Limpar leituras e gráficos anteriores
clear;
close all;

% Configuração da porta serial
portaSerial = '/dev/ttyUSB1'; % Substitua pela sua porta serial (exemplo: COM3 no Windows)
taxaBits = 115200;            % Taxa de bits definida no código Arduino

% Abre a conexão serial
s = serialport(portaSerial, taxaBits);

% Inicializa os gráficos
figure(1);
h1 = animatedline('Color', 'b', 'DisplayName', 'Referência de Velocidade');
h2 = animatedline('Color', 'r', 'DisplayName', 'Velocidade Filtrada');
h3 = animatedline('Color', 'g', 'DisplayName', 'Velocidade Não Filtrada');
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
velocidades_filtradas = [];
velocidades_nao_filtradas = [];

% Leitura e plotagem em tempo real
while datetime('now') - tempoInicial < duration
    if s.NumBytesAvailable > 0
        % Lê os valores do Arduino
        data = readline(s); % Lê uma linha da porta serial
        valores = str2double(split(data, ' ')); % Divide pelos separadores de espaço

        % Verifica se os dados são válidos
        if numel(valores) == 3
            referencia = valores(1);              % Referência de velocidade (RPM)
            velocidade_filtrada = valores(2);     % Velocidade filtrada do motor (RPM)
            velocidade_nao_filtrada = valores(3); % Velocidade não filtrada do motor (RPM)

            % Obtém o tempo atual e adiciona o ponto ao gráfico
            tempoAtual = datetime('now') - tempoInicial;
            tempoSegundos = seconds(tempoAtual);

            % Adiciona dados aos arrays
            tempos = [tempos; tempoSegundos];
            referencias = [referencias; referencia];
            velocidades_filtradas = [velocidades_filtradas; velocidade_filtrada];
            velocidades_nao_filtradas = [velocidades_nao_filtradas; velocidade_nao_filtrada];

            % Adiciona pontos aos gráficos
            addpoints(h1, tempoSegundos, referencia);
            addpoints(h2, tempoSegundos, velocidade_filtrada);
            addpoints(h3, tempoSegundos, velocidade_nao_filtrada);
            drawnow;
        end
    end
end

% Fecha a conexão serial
clear s;

% Salva os dados em um arquivo
dadosSalvos = [tempos, referencias, velocidades_filtradas, velocidades_nao_filtradas];
writematrix(dadosSalvos, 'controle_velocidade_motor.txt', 'Delimiter', '\t');

% Gera o gráfico final com os dados armazenados
figure(2);
plot(tempos, referencias, 'b', 'DisplayName', 'Referência de Velocidade');
hold on;
plot(tempos, velocidades_filtradas, 'r', 'DisplayName', 'Velocidade Filtrada');
plot(tempos, velocidades_nao_filtradas, 'g', 'DisplayName', 'Velocidade Não Filtrada');
title('Dados armazenados do controle de velocidade do motor');
xlabel('Tempo (s)');
ylabel('Velocidade (RPM)');
legend;
grid on;