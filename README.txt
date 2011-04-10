1) INFORMAZIONI GENERALI

Il "pacchetto" che vi fornisco e' strutturato in
un README.txt (questo file) ed in 3 directory,
ProxyTCP_perDebug, Sessione e Applicazioni.

- nella directory ProxyTCP_perDebug e' contenuta una
versione del proxy tcp (o ritardatore o bastardo) che puo' essere
usato per "far cadere" le connessioni a vostro piacere.
Compilandolo ed eseguendolo viene visualizzato un help minimale.

- La directory Applicazioni contiene 4 sottodirectory,
una per ogni livello di specifiche (MINIMO,BASSO,MEDIOBASSO,MEDIOALTO).
In ciascuna sottodirectory c'e' un esempio di applicazione
per il test delle librerie per quel livello di specifiche.
I Makefile assumono che la vostra libreria di sessione
e' contenuta nel modulo oggetto TCP_Session.o collocato
nella directory Sessione. Ovviamente potete modificare
a piacere il Makefile per adattarlo alle vostre scelte.
Qui di seguito (al punto 2) spiego come eseguire le applicazioni

- La directory Sessione contiene, "come esempio" lo scheletro
delle librerie, ma ovviamente queste non svolgono
il compito (gestione caduta connessione) che voi dovete realizzare, 
si limitano a richiamare le analoghe system call del TCP.

2) ESEGUIRE LE APPLICAZIONI DI TEST

In alcuni casi (minimo e basso) anche il client
in un certo momento apre un socket listening e
resta in attesa di richieste di connessioni,
quindi fate attenzione ai parametri che passate al proxy.
Mi raccomando, seguite gli esempi qui di seguito,
che chiariscono le operazioni da svolgere.

definizioni:
- IPP e' l'IP del Proxy
- IPS e' l'IP del Server
- IPC e' l'IP del Client
- il Server e' in attesa sulla Porta PortS
- il Client e' in attesa sulla Porta PortC (quando necessario)
- il Proxy e' in attesa sulle Porte PortP1 e PortP2
- il Proxy ricevera' connessioni sulla porte PortP1 dal Server
e le inoltrera' sulla porta PortC del Client
- il Proxy ricevera' connessioni sulla porte PortP2 dal Client
e le inoltrera' sulla porta PortS del Server

------------------------------------------------------
2.1) come lanciare il test per le specifiche medioalte:

sull' host IPP lanciare il Proxy
 ./tcpforward2directionthreads.exe  PortP1 PortS IPS

sull' host IPS lanciare il Server
 ./Servmedioalto.exe PortS1

sull' host IPC lanciare il Client
 ./Climedioalto.exe IPP Port1P

come caso particolare, se volete eseguire tutti i processi
su uno stesso host, si deve avere cura di non usare le stesse porte,
dunque potreste lanciarli cosi':
come caso particolare, in uno stesso host
 ./tcpforward2directionthreads.exe 7000 9000 127.0.0.1
 ./Servmedioalto.exe 9000
 ./Climedioalto.exe 127.0.0.1 7000


consiglio amichevole:
provate una prima volta ad eseguire le applicazioni
senza l'intermediazione del proxy, lanciandole sullo
stesso host cosi':
 ./Servmedioalto.exe 9000
 ./Climedioalto.exe 127.0.0.1 9000
In questo modo vedete come si comportano senza interruzioni.

successivamente, eseguite le applicazioni inserendo il proxy
sullo stesso host dei due processi Client e Server, 
una prima volta senza killare il proxy, una seconda volta killandolo.
 
infine, eseguite le applicazioni inserendo il proxy
in un host diverso da quello dei due processi Client e Server,
una prima volta senza killare il proxy, una seconda volta killandolo.

Nota bene:
le demo verranno effettuate eseguendo il proxy
su un host diverso da quello di Client e Server.
questo perche' i tempi di trasmissione reali
complicano la situazione e concorrono ad evidenziare 
eventuali magagne dello strato di sessione.


--------------------------------------------------------
2.2) come lanciare il test per le specifiche mediobasse:

sull' host IPP lanciare il Proxy
 ./tcpforward2directionthreads.exe  PortP1 PortS IPS
sull' host IPS lanciare il Server
 ./Servmediobasso.exe PortS1
sull' host IPC lanciare il Client
 ./Climediobasso.exe IPP PortP1

come caso particolare, in uno stesso host
 ./tcpforward2directionthreads.exe 7000 9000 127.0.0.1
 ./Servmediobasso.exe 9000
 ./Climediobasso.exe 127.0.0.1 7000


------------------------------------------------
2.3) come lanciare il test per le specifiche basse):

sull' host IPP lanciare il Proxy
 ./tcpforward2directionthreads.exe  PortP1 PortS IPS  PortP1+1 PortS+1 IPS
sull' host IPS lanciare il Server
 ./Servbasso.exe PortS
sull' host IPC lanciare il Client
 ./Clibasso.exe IPP PortP

ad esempio
 ./tcpforward2directionthreads.exe   7000 9000 IPS  7001 9001 IPS
 ./Servbasso.exe 9000
 ./Clibasso.exe IPP 7000

come caso particolare, in uno stesso host
 ./tcpforward2directionthreads.exe 7000 9000 127.0.0.1 7001 9001 127.0.0.1
 ./Servbasso.exe 9000
 ./Clibasso.exe 127.0.0.1 7000

------------------------------------------------
2.4) come lanciare il test per le specifiche minime:

sull' host IPP lanciare il Proxy
 ./tcpforward2directionthreads.exe  PortP1 PortS IPS  PortP1+1 PortS+1 IPS
sull' host IPS lanciare il Server
 ./Servminimo.exe PortS
sull' host IPC lanciare il Client
 ./Climinimo.exe IPP PortP

ad esempio
 ./tcpforward2directionthreads.exe   7000 9000 IPS  7001 9001 IPS
 ./Servminimo.exe 9000
 ./Climinimo.exe IPP 7000

come caso particolare, in uno stesso host
 ./tcpforward2directionthreads.exe 7000 9000 127.0.0.1 7001 9001 127.0.0.1
 ./Servminimo.exe 9000
 ./Climinimo.exe 127.0.0.1 7000


------------------------------------------------
2.5

Ri Nota bene:
Nota bene:
le demo verranno effettuate eseguendo il proxy
su un host diverso da quello di Client e Server.
questo perche' i tempi di trasmissione reali
complicano la situazione e concorrono ad evidenziare 
eventuali magagne dello strato di sessione.


------------------------------------------------


2.6) in bocca al lupo!!!


