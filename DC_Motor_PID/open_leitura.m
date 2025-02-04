% Limpar leituras e gráficos anteriores
clear;
close all;

% Nome do arquivo de dados
nomeArquivo = 'controle_velocidade_motor.txt';

% Verifica se o arquivo existe
if exist(nomeArquivo, 'file') == 2
    % Lê os dados do arquivo
    dados = readmatrix(nomeArquivo, 'Delimiter', '\t');

    % Extrai as colunas de dados
    tempos = dados(:, 1);                  % Tempo (s)
    referencias = dados(:, 2);             % Referência de velocidade (RPM)
    velocidades_filtradas = dados(:, 3);   % Velocidade filtrada (RPM)
    velocidades_nao_filtradas = dados(:, 4); % Velocidade não filtrada (RPM)

    % Plota os dados
    figure;
    plot(tempos, referencias, 'b', 'DisplayName', 'Referência de Velocidade');
    hold on;
    plot(tempos, velocidades_filtradas, 'r', 'DisplayName', 'Velocidade Filtrada');
    plot(tempos, velocidades_nao_filtradas, 'g', 'DisplayName', 'Velocidade Não Filtrada');
    hold off;

    % Configurações do gráfico
    title('Dados do Controle de Velocidade do Motor');
    xlabel('Tempo (s)');
    ylabel('Velocidade (RPM)');
    legend;
    grid on;
else
    % Mensagem de erro se o arquivo não existir
    error('Arquivo não encontrado: %s', nomeArquivo);
end