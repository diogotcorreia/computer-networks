# Lab 1 (LEICMyBox RC Labs 2016-2017)

## HTTP1.cap

A sequência de pacotes guardada no ficheiro `/mnt/alunos/HTTP1.cap` traduz o
descarregamento, a partir de um _browser_ Google Chrome, de uma página associada
ao livro _Computer Networking: A Top-Down Approach_, com o URL
`http://gaia.cs.umass.edu/kurose_ross/interactive/`.

### 2.1

**Qual a versão de HTTP usada pelo servidor `gaia.cs.umass.edu`?**

### 2.2

**Qual o endereço IP do servidor `gaia.cs.umass.edu`?**

### 2.3

**Em que formato é que o servidor `gaia.cs.umass.edu` envia o ficheiro de base?**

### 2.4

**Qual o número de bytes do ficheiro de base?**

### 2.5

**Quando é que o ficheiro `/kurose_ross/interactive/6e_cover.jpg` foi modificado
pela última vez no servidor `gaia.cs.umass.edu`?**

### 2.6

**Quantos ficheiros é que o "browser" descarrega do servidor `gaia.cs.umass.edu`?**

### 2.7

**Quantas sessões paralelas TCP é que o browser estabelece com o servidor `gaia.cs.umass.edu`?**

### 2.8

**Qual o número de ficheiros transferidos em cada sessão TCP?**

### 2.9

**É usado "pipelining" em cada uma das sessões TCP? Como pode ter a certeza?**

### 2.10

**Em quantos segmentos TCP é subdividido o ficheiro `/kurose_ross/interactive/jims_hr.jpg`?**

### 2.11

**Ainda considerando a transferência do ficheiro `/kurose_ross/interactive/jims_hr.jpg`,
o final do cabeçalho da resposta HTTP coincide com o final de um segmento TCP?
Qual o número de ordem de captura do segmento onde se pode confirmar a sua resposta?**

### 2.12

**As sessões TCP com o servidor `gaia.cs.umass.edu` são terminadas durante a captura?
Se sim, qual o evento que leva à terminação e quem toma a iniciativa de terminar
(servidor ou _browser_)?**

### 2.13

**Qual a ordem de grandeza do tempo de ida-e-volta que separa o "browser" do
servidor `gaia.cs.umass.edu`?**

## HTTP2.cap

No programa **wireshark** feche a janela com os dados anteriores (não é necessário guardar em ficheiro)
e carregue agora o ficheiro `/mnt/alunos/HTTP2.cap`.
Este ficheiro contém uma sequência de pacotes que traduz um conjunto de interações entre
um _browser_ e um servidor HTTP.
Responda às seguintes questões, para as quais lhe pode ser útil modificar o filtro
de visualização para conter apenas "`http`".

### 2.14

**Quantos pedidos são feitos pelo "browser" ao servidor?**

### 2.15

**Como justifica as diferenças entre a primeira e a segunda resposta do servidor?**

### 2.16

**Como justifica a última resposta do servidor?**

## SMTP.cap

A sequência de pacotes guardada no ficheiro `/mnt/alunos/SMTP.cap` traduz o envio de
uma mensagem de correio eletrónico composta através de um leitor, tendo o utilizador
tido acesso aos cabeçalhos `TO:`, `Cc:`, `Bcc:`, `Reply To:` e `Subject:`.
Responda às questões seguintes.

### 3.1

**Qual o nome do servidor de correio eletrónico?**

### 3.2

**Qual o porto associado ao servidor de correio eletrónico?**

### 3.3

**Qual o endereço IP do servidor de correio eletrónico?**

### 3.4

**Quais os comandos SMTP enviados pelo leitor de correio?**

### 3.5

**Quem são os destinatários deste correio eletrónico?**

### 3.6

**Com que cabeçalhos é que o utilizador terá composto a mensagem de correio eletrónico
por forma a justificar que os destinatários do comando `RCPT TO:` não coincidam
com o cabeçalho `TO:`?**

## 3.7

**De que é composta a mensagem de correio eletrónico?**

## 3.8

**Como é codificado o ficheiro `email.jpg`?**
