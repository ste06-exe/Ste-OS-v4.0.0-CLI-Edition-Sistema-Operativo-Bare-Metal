# 🚀 Ste OS v4.0.0 (CLI Edition) – Sistema Operativo Bare-Metal

**Ste OS** è un sistema operativo hobbistico/didattico sviluppato interamente da zero per architettura x86 (32-bit), capace di avviarsi ed eseguire codice direttamente sull'hardware reale (bare-metal) senza appoggiarsi a nessun ecosistema preesistente come Linux, DOS o Windows.

Questa versione **v4 CLI (Command Line Interface)** consolida una solida modalità testo e integra un set completo di utility diagnostiche e matematiche, interfacciandosi direttamente con i componenti hardware fondamentali del PC tramite porte di I/O e registri di sistema.

---

### 📊 Composizione del Codice
Il sistema adotta un approccio ibrido per massimizzare l'efficienza e il controllo dell'hardware:
* **95% C (ISO C99):** Utilizzato per l'intera logica del kernel, driver video VGA in modalità testo, gestione dell'I/O da tastiera, driver CMOS/RTC e lo sviluppo delle applicazioni integrate.
* **5% Assembly (NASM):** Dedicato alle operazioni critiche di basso livello, come l'interfacciamento con l'header Multiboot, l'inizializzazione dello Stack di sistema e le istruzioni hardware native della CPU (`cpuid`, `hlt`, `cli`).

---

### 📂 Architettura dei File Utilizzati

Il progetto è strutturato in tre file fondamentali che compongono la pipeline di build:

1. **`boot.asm` (Assembly x86 - NASM):** Il punto di ingresso del sistema. Contiene l'header Multiboot che permette a GRUB di riconoscere il kernel, configura lo stack puntando a `stack_top` e passa il controllo al codice C. Include inoltre le funzioni per fare il probing delle feature della CPU.
2. **`kernel.c` (Linguaggio C):** Il cuore pulsante di Ste OS. Gestisce il ring-buffer della tastiera US, il driver video mappato all'indirizzo fisico `0xB8000` con scorrimento e colori personalizzati, la lettura dei registri CMOS per l'orologio RTC in tempo reale e la RAM, e il loop dei sottomenu (Calcolatrice integrata, Diagnostica hardware, Info di sistema).
3. **`linker.ld` (Linker Script):** Definisce la mappa di memoria del binario finale. Dice al linker di organizzare le sezioni (`.text`, `.rodata`, `.data`, `.bss`) forzando il caricamento all'indirizzo fisico **1 MB (`0x100000`)**, standard fondamentale per i kernel Multiboot.

---

### 🛠️ Caratteristiche Principali (Main Features)

* **Bootloader standardizzato:** Pienamente compatibile con specifiche GNU GRUB Multiboot v1.
* **Kernel in Protected Mode:** Esecuzione nativa a 32-bit con controllo totale dei registri.
* **Hardware Probing autonomo:** Rilevamento in tempo reale del Vendor ID della CPU, supporto alle estensioni a 64-bit e calcolo della memoria RAM installata tramite porte CMOS.
* **Real Time Clock (RTC):** Lettura diretta dell'orologio hardware per data e ora di sistema.
* **Calcolatrice Integrata:** Sottomenu matematico robusto per operazioni fondamentali (Addizione, Sottrazione, Moltiplicazione, Divisione con calcolo del resto) a prova di errore.

---

### 💻 Schermata Informativa di Sistema
All'interno del menu principale, selezionando l'Opzione 5, il sistema risponde direttamente a schermo con i suoi dati identificativi nativi:

```text
=== INFORMAZIONI SUL SISTEMA OPERATIVO ===

Nome OS:      Ste OS
Versione:     4.0.0-Beta-2026
Sviluppatore: Stefano Fragapane
Kernel Mode:  Protected Mode a 32-bit (x86)
Bootloader:   GNU GRUB Multiboot v1 Compliant
Linguaggi:    95% C, 5% Assembly (NASM)

------------------------------------------------
Nota: Questo OS gira interamente in modalita' bare-metal
senza dipendenze da Linux o Windows.