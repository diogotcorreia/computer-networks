# Lab 1 (LEICMyBox RC Labs 2016-2017)

## HTTP1.cap

A sequência de pacotes guardada no ficheiro `/mnt/alunos/HTTP1.cap` traduz o
descarregamento, a partir de um _browser_ Google Chrome, de uma página associada
ao livro _Computer Networking: A Top-Down Approach_, com o URL
`http://gaia.cs.umass.edu/kurose_ross/interactive/`.

### 2.1

**Qual a versão de HTTP usada pelo servidor `gaia.cs.umass.edu`?**

HTTP/1.1

Enviado no frame 9

### 2.2

**Qual o endereço IP do servidor `gaia.cs.umass.edu`?**

128.119.245.12

### 2.3

**Em que formato é que o servidor `gaia.cs.umass.edu` envia o ficheiro de base?**

HTML (text/html)

### 2.4

**Qual o número de bytes do ficheiro de base?**

6554 bytes

Vem no header `Content-Type` do frame 14

### 2.5

**Quando é que o ficheiro `/kurose_ross/interactive/6e_cover.jpg` foi modificado
pela última vez no servidor `gaia.cs.umass.edu`?**

Sun, 03 Jun 2012 12:18:06 GMT

Vem no header `Last-Modified` do frame 155

### 2.6

**Quantos ficheiros é que o "browser" descarrega do servidor `gaia.cs.umass.edu`?**

4 (base HTML, default.css, jims_hr.jpg e 6e_cover.jpg)

É fácil contar aplicando o filtro `http`

### 2.7

**Quantas sessões paralelas TCP é que o browser estabelece com o servidor `gaia.cs.umass.edu`?**

2

Temos 2 pares [SYN], [SYN, ACK] (e [FIN, ACK]).
Também podemos filtrar pela stream TCP, `tcp.stream eq 0`, `tcp.stream eq 1`,
enquanto `tcp.stream eq 2` dá vazio.

Uma das sessões começa no frame 1 e outra no frame 2.
Também podemos reparar que são sessões diferentes pelo tracejado na linha à esquerda.

### 2.8

**Qual o número de ficheiros transferidos em cada sessão TCP?**

Uma das sessões transfere 3 ficheiros (base HTML, default.css e 6e_cover.jpg)
e a outra 1 ficheiro (jims_hr.jpg).

### 2.9

**É usado "pipelining" em cada uma das sessões TCP? Como pode ter a certeza?**

Não, dado que na sessão que transfere 3 ficheiros, apenas é feito o pedido
para descarregar o ficheiro (GET) após o anterior ter terminado.
Na sessão que transfere apenas um ficheiro nada podemos concluir.

### 2.10

**Em quantos segmentos TCP é subdividido o ficheiro `/kurose_ross/interactive/jims_hr.jpg`?**

13

Ao ir ao frame 39, diz quantos "Reassembled TCP Segments" compõem a mensagem.

### 2.11

**Ainda considerando a transferência do ficheiro `/kurose_ross/interactive/jims_hr.jpg`,
o final do cabeçalho da resposta HTTP coincide com o final de um segmento TCP?
Qual o número de ordem de captura do segmento onde se pode confirmar a sua resposta?**

Não, o segmento TCP em que termina o cabeçalho HTTP contém bytes da
imagem após o cabeçalho HTTP. Pode-se confirmar isto no frame 23.

### 2.12

**As sessões TCP com o servidor `gaia.cs.umass.edu` são terminadas durante a captura?
Se sim, qual o evento que leva à terminação e quem toma a iniciativa de terminar
(servidor ou _browser_)?**

Sim, ambas as sessões são terminadas durante a captura.
O servidor envia nas suas respostas o header `Keep-Alive: timeout=5 ...`,
que indica que irá fechar a conexão 5 segundos após não receber nenhuma
mensagem do browser.
Então, 5 segundos após enviar os ficheiros que lhe foram pedidos, o servidor
toma a iniciativa de terminar a ligação, através do envio de um [FIN, ACK].

### 2.13

**Qual a ordem de grandeza do tempo de ida-e-volta que separa o "browser" do
servidor `gaia.cs.umass.edu`?**

Decisegundos (1/10 segundos)

## HTTP2.cap

No programa **wireshark** feche a janela com os dados anteriores (não é necessário guardar em ficheiro)
e carregue agora o ficheiro `/mnt/alunos/HTTP2.cap`.
Este ficheiro contém uma sequência de pacotes que traduz um conjunto de interações entre
um _browser_ e um servidor HTTP.
Responda às seguintes questões, para as quais lhe pode ser útil modificar o filtro
de visualização para conter apenas "`http`".

### 2.14

**Quantos pedidos são feitos pelo "browser" ao servidor?**

3

Frames 4, 8 e 11

### 2.15

**Como justifica as diferenças entre a primeira e a segunda resposta do servidor?**

Na segunda resposta, o browser envia o HTTP header If-Modified-Since, que diz
ao servidor que existe uma versão em cache modificada numa certa data.
Como o servidor sabe que a página não foi alterada desde essa mesma data,
responde ao browser com 304 Not Modified, indicando ao browser que a página não
foi alterada, evitando enviá-la novamente.
Tal não acontece na primeira resposta, visto que o browser não inclui o header
If-Modified-Since nesse pedido.

### 2.16

**Como justifica a última resposta do servidor?**

O browser não inclui o header If-Modified-Since, pelo que o servidor não sabe
se o browser tem ou não o ficheiro em cache. Por essa razão, o servidor envia
o ficheiro pedido para o browser (200 OK).

## SMTP.cap

A sequência de pacotes guardada no ficheiro `/mnt/alunos/SMTP.cap` traduz o envio de
uma mensagem de correio eletrónico composta através de um leitor, tendo o utilizador
tido acesso aos cabeçalhos `TO:`, `Cc:`, `Bcc:`, `Reply To:` e `Subject:`.
Responda às questões seguintes.

### 3.1

**Qual o nome do servidor de correio eletrónico?**

cascais.lx.it.pt

Vem no frame 4

### 3.2

**Qual o porto associado ao servidor de correio eletrónico?**

Porto 25

No frame 1, o cliente na porta 60142 fala com o servidor na porta 25

### 3.3

**Qual o endereço IP do servidor de correio eletrónico?**

193.136.221.1

Destino do frame 1

### 3.4

**Quais os comandos SMTP enviados pelo leitor de correio?**

ELHO (frame 5)
MAIL FROM: (frame 8)
RCPT TO: (frames 10, 12 e 14)
DATA (frame 16)
QUIT (frame 32)

### 3.5

**Quem são os destinatários deste correio eletrónico?**

sanguino@lx.it.pt
paulo.lobato.correia@tecnico.ulisboa.pt
joao.sobrinho@lx.it.pt

Frames 10, 12 e 14, respetivamente

### 3.6

**Com que cabeçalhos é que o utilizador terá composto a mensagem de correio eletrónico
por forma a justificar que os destinatários do comando `RCPT TO:` não coincidam
com o cabeçalho `TO:`?**

O utilizador utilizou os cabeçalhos TO e Bcc.
No TO colocou joao.sobrinho@lx.it.pt e no Bcc colocou sanguino@lx.it.pt
e paulo.lobato.correia@tecnico.ulisboa.pt. Os destinatários colocados em Bcc
não são incluídos na mensagem.

## 3.7

**De que é composta a mensagem de correio eletrónico?**

A mensagem é composta pelos cabeçalhos (From, Subject, Reply-To, To, Organization,
Message-ID, Date, User-Agent, MIME-Version e Content-Type), assim como
o conteúdo que consiste em texto (text/plain) e uma imagem email.jpg (image/jpg)
de 12118 bytes em anexo.

## 3.8

**Como é codificado o ficheiro `email.jpg`?**

O ficheiro é codificado em Base64, tal como indicado pelo header Content-Transfer-Encoding.
