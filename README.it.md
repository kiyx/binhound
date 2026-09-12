# BinHound

> Nome provvisorio. Un generatore statico di **SBOM / CBOM** e verificatore di prontezza **CRA** per binari compilati e firmware.

**Stato: sviluppo iniziale. Non funziona ancora nulla. Il progetto viene costruito in pubblico, un passo alla volta.**

[English](README.md) | Italiano

## Cosa fa

BinHound prende un file eseguibile di cui non esiste il codice sorgente (un binario compilato con i simboli rimossi, un firmware) e risponde a tre domande:

1. **Cosa c'è dentro?** Identifica i componenti di terze parti (librerie, strumenti) e le loro versioni.
2. **Quale crittografia usa?** Inventaria algoritmi, protocolli e certificati (CBOM).
3. **Cosa è pericoloso?** Confronta i componenti trovati con i database pubblici di vulnerabilità (CVE / OSV).

I risultati vengono esportati come documenti standard:

- **SBOM** in formato CycloneDX (la lista degli ingredienti software)
- **CBOM** in formato CycloneDX (`cryptoProperties`)
- **VEX** (quali vulnerabilità pubblicate riguardano davvero il prodotto)
- un **report leggibile** con la checklist di prontezza al CRA

Ogni risultato porta con sé un'**evidenza** e un **livello di confidenza** (alta / media / bassa). Un singolo indizio debole non produce mai un risultato ad alta confidenza.

## Perché

- Il **Cyber Resilience Act europeo** (Regolamento 2024/2847) obbliga i produttori che vendono in UE a conoscere e dichiarare i componenti software dei loro prodotti. Gli obblighi di segnalazione iniziano l'**11 settembre 2026**; la conformità completa è richiesta entro l'**11 dicembre 2027**. Le multe arrivano a 15 milioni di euro o al 2,5% del fatturato globale.
- Ospedali e committenti pubblici rifiutano sempre più spesso prodotti senza SBOM: il 35% lo indica come requisito di acquisto.
- La migrazione alla **crittografia post-quantistica** richiede di sapere in anticipo dove vengono usati RSA ed ECC. Non puoi migrare ciò che non sai inventariare. Banche e pubbliche amministrazioni hanno già iniziato questo lavoro.

## Cosa non è

- Non è un antivirus né uno scanner di malware.
- Non è uno strumento di exploitation o penetration testing.
- Non è un decompilatore: non ricostruisce il codice sorgente.
- Non esegue mai il file analizzato.
- Non promette completezza. L'analisi dei binari ha limiti oggettivi; lo strumento li documenta invece di nasconderli.
- Non tratta mai "nessun risultato" come "nessun rischio": ogni report dichiara quanta parte del file è stata analizzabile.

## Esempio (output previsto, non ancora implementato)

```
$ binhound scan firmware.bin --format cyclonedx --output sbom.json

BinHound 0.1.0 - analisi statica dei componenti
File      : firmware.bin (ELF 64-bit LSB executable, x86-64, stripped)
Scansionato: 1.2 MB in 0.42 s

Componenti (12 trovati)
  openssl   3.0.8     alta     stringa "OpenSSL 3.0.8" a 0x000a1c30
  zlib      1.2.11    alta     stringa "inflate 1.2.11"  a 0x000b0431
  busybox   1.35.0    media    3 corrispondenze nei simboli
  ...

Vulnerabilità (4 trovate: 1 critica, 2 alte, 1 media)
  CVE-2023-0286   openssl 3.0.8   critica
  ...

Prontezza CRA
  [ok]    SBOM generato
  [warn]  Meccanismo di aggiornamento non verificabile dal binario
  [todo]  Politica di segnalazione vulnerabilità non presente nell'artefatto

Output: sbom.json (CycloneDX 1.6, valido secondo lo schema)
```

## Come funziona

```
 file di input
     |
     v
+------------+    +-------------+    +----------------+    +-------------+
|  Parser    | -> |  Rilevamento| -> |  Confronto     | -> |   Report    |
| ELF (v0.1) |    | firme       |    | CVE / OSV      |    | CDX, VEX    |
| PE, FW     |    | + evidenze  |    | + gravità      |    | + check CRA |
+------------+    +-------------+    +----------------+    +-------------+
```

1. **Parsing**: il file viene letto come sequenza di byte; header, sezioni e tabelle dei simboli vengono decodificati. Nulla viene eseguito.
2. **Rilevamento**: un database di firme collega stringhe, nomi di simboli e costanti binarie a componenti, versioni e pesi di confidenza. L'identificazione del componente e l'attribuzione della versione sono passi separati; ogni corrispondenza viene registrata come evidenza.
3. **Confronto**: i componenti e le versioni rilevate vengono confrontati con i database di vulnerabilità.
4. **Report**: i risultati vengono emessi come documenti CycloneDX SBOM, CBOM e VEX, più un report di conformità leggibile.

## Struttura del repository (prevista)

```
binhound/
├── CMakeLists.txt
├── src/
│   ├── cli/          # interfaccia a riga di comando
│   ├── parser/       # formati binari (prima ELF)
│   ├── detect/       # motore firme + confidenza + evidenze
│   ├── match/        # confronto vulnerabilità (OSV / cache locale)
│   ├── report/       # CycloneDX SBOM, CBOM, VEX, report leggibile
│   └── util/         # byte, endianness, stringhe, errori
├── data/
│   └── signatures/   # database delle firme dei componenti
├── tests/
│   └── fixtures/     # piccoli binari dal contenuto noto
└── docs/
    ├── REQUIREMENTS.md
    ├── ROADMAP.md
    ├── RISKS.md
    ├── FORGE.md
    ├── LIMITATIONS.md
    └── STUDY-PLAN.it.md
```

## Riepilogo requisiti

L'elenco completo è in [docs/REQUIREMENTS.md](docs/REQUIREMENTS.md). In breve:

- C++20, CMake, dipendenze esterne minime (nessuna dipendenza runtime obbligatoria)
- Prima Linux x86-64; Windows e macOS in seguito
- Prima ELF (v0.1): 32/64 bit, little e big-endian, indipendente dall'architettura (x86-64, ARM, MIPS); PE e firmware in seguito
- Precisione e copertura misurate e pubblicate a ogni release; ogni regola ha fixture positiva e negativa
- Scorecard di copertura in ogni output: cosa è stato analizzabile e cosa no
- Output deterministici e validi secondo lo schema
- Parsing sicuro di input non fidato; nessun crash su file malformati
- CI con test, sanitizzatori, warning come errori e validazione dello schema

## Roadmap

I dettagli sono in [docs/ROADMAP.md](docs/ROADMAP.md).

| Versione | Obiettivo |
| :--- | :--- |
| v0.1 | Scheletro funzionante: parsing ELF, rilevamento a firme, SBOM CycloneDX, CLI, test, CI |
| v0.2 | CBOM + metadati embedded (Go/Rust) + forge delle firme |
| v0.3 | Confronto vulnerabilità (OSV) + VEX + punteggio di confidenza |
| v0.4 | Report di prontezza CRA |
| v0.5 | Supporto PE + firmware gestito con tool esterni (binwalk/unblob) |
| v0.6 | Modulo sanitario (DICOM, solo su sistemi autorizzati) |
| v1.0 | Formato database stabile, pacchetti, fuzzing, benchmark |

Le versioni sono obiettivi, non promesse. Ogni milestone si chiude con una release taggata e documentazione aggiornata.

## Compilazione (prevista, non ancora implementata)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/bin/binhound scan /bin/ls --format cyclonedx --output sbom.json
```

## Contribuire

- Il database delle firme è progettato per accettare contributi senza conoscere il C++.
- Le issue "good first issue" verranno pubblicate con la prima release di codice.
- Prima di contribuire, leggi [docs/REQUIREMENTS.md](docs/REQUIREMENTS.md) e [docs/ROADMAP.md](docs/ROADMAP.md).

## Licenza

**Apache-2.0.** Tutto ciò che determina i risultati — regole, calcolo della confidenza, evidenze — è aperto e ispezionabile: per uno strumento il cui valore è un output verificabile dall'auditor, una logica di rilevamento chiusa sarebbe una contraddizione. Se in futuro esisterà un modello di business, vivrà fuori dal codice: supporto, consulenza, formazione.
