% Ola emacs, isto e -*- matlab -*-
% $Id$
% Andre Rabello <Andre.Rabello@ufrj.br>

% Este script le os dados de um dicionario e gera uma sequencia de
% tamanho dado por `N' com os caracteres do dicionario em ordem
% aleatoria. Para realizar este processo, cria-se, inicialmente um
% vetor cujo o tamanho e a contagem total dos simbolos no dicionario,
% contendo os simbolos repetidos. Depois, embaralha-se o dicionario
% usando-se `randperm'. Aplica-se `randperm' gerando novas
% sequencias, tanto quanto se deseje aumentar a sequencia.

dict='bcorpus';
N=50000;

fprintf(2,'Lendo dicionario...');
fid=fopen(strcat(dict,'.dict'),'r');
data = -1;
while 1
  line=fgetl(fid);
  vals=double(line);
  if (length(line) > 1 & vals(1) ~= '#'),
    [A]=sscanf(line,'%lu %lu',[1 2]); %% dados normais
    if data == -1,
      data = A(1)*ones(1,A(2));
    else
      data = [data A(1)*ones(1,A(2))];
    end
    %%fprintf(2, 'Simbolo %lu lido, contando %lu\n',A(1),A(2));
  end
  if line == -1, break; end
end
fclose(fid);
fprintf(2,'feito!\n');
	    
fprintf(2,'Calculando mensagem...');
r = -1;
if N > length(data);
  iter=fix(N/length(data));
  % o resultado
  for i=1:iter,
    if r==-1, r = data(randperm(length(data)));
    else r = [r data(randperm(length(data)))];
    end
  end
end

% complementa
comp = rem(N,length(data));
need = data(randperm(length(data)));
if r ~= -1, r = [r need(1:comp)];
else r = need(1:comp);
end
clear comp need data line vals N A iter i;
fprintf(2,'feito!\n');

% escreve em disco
fprintf(2, 'Escrevendo dados no disco (%s)...', strcat(dict,'.mesg'));
fid = fopen(strcat(dict,'.mesg'),'w');
fprintf(fid,'%lu\n',r);
fclose(fid);
fprintf(2, 'feito!\n');

% testa o que esta escrito
fprintf(2, 'Testando arquivo de mensagem...');
fid = fopen(strcat(dict,'.mesg'), 'r');
r = fscanf(fid, '%lu', [1 Inf]);
hist(r,80);
axis tight;
xlabel('Simbolos');
ylabel('Contagem para a mensagem');
title('Contagem de simbolos para a mensagem atual');
fclose(fid);
clear r dict fid;
fprintf(2, 'feito!\n');
