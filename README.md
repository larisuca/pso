# Remote Logging System

## Materie
Proiectarea Sistemelor de Operare (PSO)

**Autori:** Opincă Larisa, Rizea Amalia

---

## Descriere generală

Remote Logging System este o aplicație client–server multithreaded care colectează mesaje de tip log (INFO, WARN, ERROR) de la mai multe aplicații client și le salvează centralizat într-un fișier XML. O interfață grafică (GTK) permite vizualizarea, filtrarea și actualizarea automată a acestor loguri în timp real.

Scopul proiectului este demonstrarea conceptelor fundamentale din Proiectarea Sistemelor de Operare:
- procese concurente
- comunicație inter-proces (IPC)
- thread-uri și sincronizare (pthread mutex)
- lucrul cu fișiere
- programare bazată pe evenimente (GUI)

---

## Concepte PSO demonstrate

| Concept        | Descriere |
|----------------|-----------|
| Procese        | Fiecare client rulează ca proces separat |
| Thread-uri     | Serverul creează un thread pentru fiecare client conectat |
| Sincronizare   | `pthread_mutex_t` protejează scrierea în fișierul comun XML |
| IPC            | Comunicarea între client și server se face prin socket TCP |
| Concurență     | Mai mulți clienți pot trimite loguri simultan |
| Persistență    | Logurile sunt stocate în fișiere (locale și centralizat în XML) |
| Programare reactivă | Interfața GTK folosește evenimente și timer periodic (auto-refresh) |

---

## 1. Client

Fișier propus: `client.c`

Descriere succintă:
- Simulează o aplicație care generează periodic loguri.
- Fiecare client are un nume (ex. App1, App2, App3).
- Trimit loguri prin TCP către server și le salvează și local în `logs/AppX.log`.

Formatul unui log (text):
[YYYY-MM-DD HH:MM:SS] [AppX] [LEVEL]: Mesaj

### Funcții cheie:
- `generate_log()` – generează un mesaj random (INFO/WARN/ERROR)
- `write_local_log()` – scrie logul în fișierul local `logs/AppX.log`
- `send()` – transmite logul la server prin socket

### Concepte utilizate:
- procese independente
- seed unic: `srand(time(NULL) ^ getpid())`
- socket TCP (AF_INET, SOCK_STREAM)
---

## 2. Server
### Fișier: `server.c`

### Descriere:
- Primește conexiuni de la mai mulți clienți simultan.
- Creează câte un thread pentru fiecare client conectat.
- Fiecare thread citește mesajele primite și le scrie în `logs.xml`.

### Funcții cheie:
- `handle_client()` – rulează în fiecare thread, citește logurile clientului.
- `write_log_to_xml()` – parsează logul și îl scrie formatat în XML.
- `pthread_mutex_lock()` / `pthread_mutex_unlock()` – asigură scrierea sincronizată.

### Format XML rezultat:
```xml
<logs>
  <log>
      <timestamp>2025-10-26 19:44:31</timestamp>
      <app>App1</app>
      <level>ERROR</level>
      <message>Disk space low</message>
  </log>
</logs>
```
Concepte utilizate:
 - socket TCP (server)
 - thread-uri (pthread_create)
 - sincronizare (mutex)

---

## 3. Interfața grafică (GUI)

Fișier : `gui.c` (GTK3)
Descriere:
 - Citește fișierul logs.xml și afișează logurile colorat.
 - Permite filtrare după:
    - nivel (INFO, WARN, ERROR)
    - text introdus de utilizator
 - Se actualizează automat la fiecare 2 secunde.

Funcții cheie:

 - filter_logs() – citește fișierul XML și aplică filtrul curent.
 - on_button_clicked() – filtrează după nivel.
 - on_search() – filtrează după textul introdus.
 - auto_refresh() – actualizează automat interfața.
 - g_timeout_add_seconds(2, auto_refresh, NULL) – eveniment periodic GTK.
Colorare rânduri după nivel:
   - INFO — verde
   - WARN — portocaliu
   - ERROR — roșu
---

## Compilare și rulare

1. Compilare:
```bash
gcc server.c -o server
gcc client.c -o client
gcc gui.c -o gui `pkg-config --cflags --libs gtk+-3.0`
```

2. Pornire server:
```bash
./server
# Output de exemplu:
# Server (multithreaded) pornit pe portul 9000...
```

3. Pornire clienți (exemplu în background):
```bash
./client App1 &
./client App2 &
./client App3 &
```

4. Pornire GUI:
```bash
./gui
```

---

## Structura proiectului (exemplu)
```
RemoteLoggingSystem/
│
├── client.c
├── server.c
├── gui.c
├── README.md
├── logs.xml
└── logs/
    ├── App1.log
    ├── App2.log
    └── App3.log
```

---

## Concluzie
Proiectul Remote Logging System implementează un sistem complet de logare distribuită.
Demonstrează aplicarea practică a conceptelor fundamentale de sistem de operare: procese, thread-uri, sincronizare, comunicație inter-proces și lucrul cu fișiere.
Interfața GTK adaugă un strat vizual modern, permițând monitorizarea în timp real a evenimentelor din sistem.
