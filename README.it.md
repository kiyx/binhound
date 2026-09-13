# BinHound

> Analisi statica di binari compilati: inventaria i componenti software e, in seguito, la
> crittografia e le vulnerabilita' note, con evidenza e confidenza.

[English](README.md) | Italiano

[![CI](https://github.com/kiyx/binhound/actions/workflows/ci.yml/badge.svg)](https://github.com/kiyx/binhound/actions/workflows/ci.yml)
[![CodeQL](https://github.com/kiyx/binhound/actions/workflows/codeql.yml/badge.svg)](https://github.com/kiyx/binhound/actions/workflows/codeql.yml)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

**Stato: sviluppo iniziale.** E' implementata solo l'analisi dell'header ELF; rilevamento dei
componenti, export SBOM e il resto sono in corso. Costruito in pubblico, un passo alla volta.

## Cosa fa

Dato un binario compilato, senza codice sorgente disponibile, BinHound risponde a tre domande:

1. **Cosa c'e' dentro?** Componenti e versioni, ognuno con evidenza e livello di confidenza.
2. **Quale crittografia usa?** Algoritmi, protocolli e certificati (in programma).
3. **Cosa e' pericoloso?** Vulnerabilita' note, riportate separate dall'inventario (in programma).

I risultati vengono esportati come documenti standard CycloneDX (SBOM, CBOM, VEX) piu' un report
leggibile. Ogni report dichiara il **livello di copertura**: quanta parte del file e' stata
davvero analizzabile.

## Cosa non e'

Non e' un antivirus, non e' uno strumento di exploitation, non e' un decompilatore. Non esegue mai
il file analizzato e non tratta mai "nessun risultato" come "nessun rischio".

## Uso

```bash
binhound scan /bin/ls
binhound --version
binhound --help
```

Output attuale:

```
File:       /bin/ls
Class:      ELF64
Endianness: little
Type:       DYN
Machine:    x86-64
Entry:      0x6d30
Sections:   31
```

Exit code: `0` successo, `2` errore.

## Compilazione

Requisiti: CMake 3.28 o superiore, compilatore C++20 (GCC 13+, Clang 18+), Ninja.

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug

cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix "$HOME/.local"
```

Preset: `debug`, `relwithdebinfo`, `release` (LTO e hardening), `bench`, `asan`, `ci`.
Gli stessi controlli girano in CI su Linux, Windows e macOS, con sanitizer, clang-tidy,
clang-format, coverage e CodeQL.

## Struttura

```
binhound/
├── src/
│   ├── cli/          # punto di ingresso a riga di comando
│   ├── util/         # lettura file, interi con endianness, errori
│   └── parser/elf/   # parsing dell'header ELF
├── tests/
│   ├── unit/         # test unitari (doctest)
│   └── fixtures/     # binari di prova generati
└── data/signatures/  # firme dei componenti (in corso)
```

## Roadmap

- **v0.1** - parsing ELF, estrazione stringhe e simboli, rilevamento a firme, SBOM CycloneDX,
  coverage scorecard, output testuale colorato.
- **v0.2** - inventario crittografico (CBOM), metadati Go/Rust, database firme automatico.
- **v0.3** - confronto vulnerabilita' (OSV) e VEX.
- **v0.4** - report di prontezza CRA.
- **Dopo** - supporto PE e firmware, modulo sanitario (DICOM).

## Contribuire

Segnalazioni di bug, piccole correzioni, test e proposte di funzionalita' sono benvenuti. Apri
una issue per discutere un'idea prima di lavorarci; vedi [CONTRIBUTING.md](CONTRIBUTING.md) per
flusso di lavoro, gate di qualita' e convenzioni.

## Principi di progetto

- **Prima l'evidenza.** Ogni risultato dice perche' e' stato rilevato e con quanta confidenza.
- **Copertura onesta.** I report dichiarano cosa non e' stato analizzabile; una copertura bassa
  e' un risultato, non un fallimento.
- **Aperto e ispezionabile.** Apache-2.0, nessuna logica di rilevamento chiusa.
- **Sicuro per impostazione predefinita.** Nessuna esecuzione del file, nessun accesso alla rete
  se non richiesto esplicitamente.

## Licenza

Apache-2.0. Vedi [LICENSE](LICENSE).
