// kernel.c - Ste OS v4.0.0 (CLI Edition)

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

unsigned short *vga_buffer = (unsigned short *)VGA_ADDRESS;
int cursor_x = 0;
int cursor_y = 0;
unsigned char current_color = 0x0F; // Bianco su sfondo Nero

extern void detect_cpu(char *buffer);
extern void detect_cpu_features(unsigned int *edx, unsigned int *ecx);

// --- DRIVER PORTE HARDWARE I/O ---
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// --- DRIVER VIDEO VGA TESTO ---
void update_cursor() {
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (unsigned short)' ' | (unsigned short)current_color << 8;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void scroll_screen() {
    if (cursor_y >= VGA_HEIGHT) {
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
        }
        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            vga_buffer[i] = (unsigned short)' ' | (unsigned short)current_color << 8;
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (unsigned short)' ' | (unsigned short)current_color << 8;
        }
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (unsigned short)c | (unsigned short)current_color << 8;
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    scroll_screen();
    update_cursor();
}

void print_string(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i]);
    }
}

void print_int(int num) {
    if (num == 0) {
        print_char('0');
        return;
    }
    if (num < 0) {
        print_char('-');
        num = -num;
    }
    char buf[12];
    int i = 0;
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0) {
        print_char(buf[--i]);
    }
}

void print_bcd(unsigned char bcd) {
    int val = (bcd >> 4) * 10 + (bcd & 0x0F);
    if (val < 10) print_char('0');
    print_int(val);
}

// --- DRIVER TASTIERA ---
const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' '
};

char get_key() {
    while (1) {
        if ((inb(0x64) & 1) == 1) {
            unsigned char scancode = inb(0x60);
            if (!(scancode & 0x80)) {
                return kbd_us[scancode];
            }
        }
    }
}

void read_string(char *buffer, int max_len) {
    int i = 0;
    while (1) {
        char c = get_key();
        if (c == '\n') {
            print_char('\n');
            break;
        } else if (c == '\b' && i > 0) {
            i--;
            print_char('\b');
        } else if (c >= 32 && c <= 126 && i < max_len - 1) {
            buffer[i++] = c;
            print_char(c);
        }
    }
    buffer[i] = '\0';
}

int atoi(const char *str) {
    int res = 0;
    int sign = 1;
    int i = 0;
    if (str[0] == '-') {
        sign = -1;
        i++;
    }
    for (; str[i] != '\0'; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            res = res * 10 + (str[i] - '0');
        }
    }
    return res * sign;
}

// --- DRIVER OROLOGIO HARDWARE (CMOS RTC) ---
unsigned char get_update_in_progress_flag() {
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

unsigned char get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

// --- APPLICAZIONI INTEGRATE ---
void app_hello() {
    clear_screen();
    current_color = 0x0B; // Ciano chiaro
    print_string("=== APPLICAZIONE: HELLO WORLD ===\n\n");
    current_color = 0x0F;
    print_string("Hello, World! Benvenuto in Ste OS.\n");
    print_string("Il kernel a 32-bit sta eseguendo codice nativo direttamente sull'hardware.\n\n");
    print_string("Premi un tasto per tornare al menu...");
    get_key();
}

void app_saluto() {
    char nome[30];
    clear_screen();
    current_color = 0x0D; // Magenta chiaro
    print_string("=== APPLICAZIONE: SALUTO UTENTE ===\n\n");
    current_color = 0x0F;
    print_string("Inserisci il tuo nome: ");
    read_string(nome, 30);
    
    current_color = 0x0E; // Giallo
    print_string("\nCiao ");
    print_string(nome);
    print_string(", benvenuto nel tuo sistema operativo!\n\n");
    current_color = 0x0F;
    print_string("Premi un tasto per tornare al menu...");
    get_key();
}

void app_calcolatrice() {
    char buf1[15], buf2[15];
    clear_screen();
    current_color = 0x0E; // Giallo
    print_string("=== CALCOLATRICE INTEGRATA ===\n\n");
    current_color = 0x0F;

    print_string("Inserisci il primo numero: ");
    read_string(buf1, 15);
    int n1 = atoi(buf1);

    print_string("Scegli operazione (1: +, 2: -, 3: *, 4: /): ");
    char op = get_key();
    print_char(op);
    print_string("\n");

    print_string("Inserisci il secondo numero: ");
    read_string(buf2, 15);
    int n2 = atoi(buf2);

    print_string("\nRisultato: ");
    current_color = 0x0A; // Verde chiaro
    if (op == '1') {
        print_int(n1 + n2);
    } else if (op == '2') {
        print_int(n1 - n2);
    } else if (op == '3') {
        print_int(n1 * n2);
    } else if (op == '4') {
        if (n2 == 0) {
            current_color = 0x0C; // Rosso chiaro
            print_string("Errore! Impossibile dividere per zero.");
        } else {
            print_int(n1 / n2);
            print_string(" (Resto: ");
            print_int(n1 % n2);
            print_string(")");
        }
    } else {
        current_color = 0x0C;
        print_string("Operazione non valida.");
    }

    current_color = 0x0F;
    print_string("\n\nPremi un tasto per tornare al menu...");
    get_key();
}

void app_hardware() {
    char cpu_vendor[13];
    unsigned int feat_edx = 0, feat_ecx = 0;

    clear_screen();
    current_color = 0x0B; // Ciano chiaro
    print_string("=== DIAGNOSTICA HARDWARE DI SISTEMA ===\n\n");
    current_color = 0x0F;

    // 1. CPU Probing
    detect_cpu(cpu_vendor);
    detect_cpu_features(&feat_edx, &feat_ecx);

    print_string("Produttore CPU:   ");
    current_color = 0x0E;
    print_string(cpu_vendor);
    print_string("\n");
    current_color = 0x0F;

    print_string("Supporto 64-bit:  ");
    if (feat_edx & (1 << 29)) {
        current_color = 0x0A;
        print_string("Si (Architettura x86_64 compatibile)\n");
    } else {
        current_color = 0x0C;
        print_string("No (Solo 32-bit)\n");
    }
    current_color = 0x0F;

    // 2. Calcolo RAM da CMOS
    outb(0x70, 0x30);
    unsigned char mem_low = inb(0x71);
    outb(0x70, 0x31);
    unsigned char mem_high = inb(0x71);
    int ram_mb = (mem_low | (mem_high << 8)) / 1024 + 1;

    print_string("Memoria RAM base: ");
    current_color = 0x0E;
    print_int(ram_mb);
    print_string(" MB rilevate via CMOS\n\n");
    current_color = 0x0F;

    // 3. Orologio Hardware RTC
    while (get_update_in_progress_flag());
    unsigned char sec = get_rtc_register(0x00);
    unsigned char min = get_rtc_register(0x02);
    unsigned char ora = get_rtc_register(0x04);
    unsigned char gio = get_rtc_register(0x07);
    unsigned char mes = get_rtc_register(0x08);
    unsigned char ann = get_rtc_register(0x09);

    print_string("[Orologio Hardware CMOS RTC]\n");
    print_string("Data: ");
    print_bcd(gio); print_char('/');
    print_bcd(mes); print_string("/20");
    print_bcd(ann); print_string("\n");

    print_string("Ora:  ");
    print_bcd(ora); print_char(':');
    print_bcd(min); print_char(':');
    print_bcd(sec); print_string(" (UTC)\n\n");

    print_string("Premi un tasto per tornare al menu...");
    get_key();
}

void app_info() {
    clear_screen();
    current_color = 0x0A; // Verde chiaro
    print_string("=== INFORMAZIONI SUL SISTEMA OPERATIVO ===\n\n");
    current_color = 0x0F;
    
    print_string("Nome OS:      Ste OS\n");
    print_string("Versione:     4.0.0-Beta-2026\n");
    print_string("Sviluppatore: Stefano Fragapane\n");
    print_string("Kernel Mode:  Protected Mode a 32-bit (x86)\n");
    print_string("Bootloader:   GNU GRUB Multiboot v1 Compliant\n");
    print_string("Linguaggi:    95% C, 5% Assembly (NASM)\n");
    print_string("\n------------------------------------------------\n");
    print_string("Nota: Questo OS gira interamente in modalita' bare-metal\n");
    print_string("senza dipendenze da Linux o Windows.\n\n");

    print_string("Premi un tasto per tornare al menu...");
    get_key();
}

// --- ENTRY POINT DEL KERNEL ---
void kernel_main() {
    while (1) {
        clear_screen();
        current_color = 0x1F; // Testo bianco su barra blu
        print_string("                STE OS v4.0.0 (CLI EDITION) - MENU PRINCIPALE                 \n");
        current_color = 0x0F; // Ripristina bianco su nero
        print_string("\nSeleziona un'opzione premendo il tasto corrispondente:\n\n");
        print_string("  [1] Hello World\n");
        print_string("  [2] Saluto Utente\n");
        print_string("  [3] Calcolatrice Integrata\n");
        print_string("  [4] Diagnostica Hardware (CPU, RAM, Orologio RTC)\n");
        print_string("  [5] Informazioni sul Sistema Operativo\n");
        print_string("\n--------------------------------------------------------------------------------\n");
        print_string("In attesa di comando > ");

        char scelta = get_key();
        if (scelta == '1')      app_hello();
        else if (scelta == '2') app_saluto();
        else if (scelta == '3') app_calcolatrice();
        else if (scelta == '4') app_hardware();
        else if (scelta == '5') app_info();
    }
}