#define MSTATUS_MIE (1 << 3)
#define HALT __asm__ volatile (".word 0x00000077")
#define START_VAR_LOC 0x2000

typedef enum {
    TETRIMO_I = 0,
    TETRIMO_J,
    TETRIMO_L,
    TETRIMO_O,
    TETRIMO_S,
    TETRIMO_T,
    TETRIMO_Z
} TetriminoType;

typedef struct {
    int type;
    int color;
} Tetrimino;

volatile int* const ps2Data = (volatile int*)0x40000000;
volatile int* const ps2Stat = (volatile int*)0x40000004;
volatile int* const ps2Cont = (volatile int*)0x40000008;

volatile int* const vgaData = (volatile int*)0x50000000;
volatile int* const vgaStat = (volatile int*)0x50000004;
volatile int* const vgaCont = (volatile int*)0x50000008;
volatile int* const vgaAddr = (volatile int*)0x5000000C;
volatile int* const vgaColor = (volatile int*)0x50000010;
volatile int* const vgaShape = (volatile int*)0x50000014;

volatile int* const hexData = (volatile int*)0x60000000;

typedef struct {
    volatile int shapeNext;      // offset 0x00
    volatile int g_rng_state;    // offset 0x04
    volatile int index;          // offset 0x08
    volatile int bag[7];         // offset 0x0C
    volatile int colors[7];      // offset 0x28
    volatile int shapes[7];      // offset 0x44
    volatile int dir;            // offset 0x60
    volatile int row;            // offset 0x64
    volatile int col;            // offset 0x68
    volatile int key;            // offset 0x6C
    volatile int matrix[200];    // offset 0x70
    volatile int shapeMask[16];  // offset 0x390
    volatile int start;          // offset 0x3D0
    volatile int shapeTemp[8];   // offset 0x3D4
    volatile int matrixPrintIndex; 
    volatile int current_grid[16];
    volatile int rotated_grid[16];
    volatile int current_grid_temp[16];
    volatile int points;
} TetrisMemoryMap;

volatile int* const current_grid_ptr = (volatile int*)(START_VAR_LOC + 0x3F8);

#define GAME_DATA ((TetrisMemoryMap*)START_VAR_LOC)
#define shapeNext   (GAME_DATA->shapeNext)
#define g_rng_state (GAME_DATA->g_rng_state)
#define index       (GAME_DATA->index)
#define bag         (GAME_DATA->bag)
#define colors      (GAME_DATA->colors)
#define shapes      (GAME_DATA->shapes)
#define dir         (GAME_DATA->dir)
#define row         (GAME_DATA->row)
#define col         (GAME_DATA->col)
#define key         (GAME_DATA->key)
#define matrix      (GAME_DATA->matrix)
#define shapeMask   (GAME_DATA->shapeMask)
#define start       (GAME_DATA->start)
#define shapeTemp   (GAME_DATA->shapeTemp)
#define matrixPrintIndex   (GAME_DATA->matrixPrintIndex)
#define current_grid   (GAME_DATA->current_grid)
#define rotated_grid   (GAME_DATA->rotated_grid)
#define current_grid_temp   (GAME_DATA->current_grid_temp)
#define points   (GAME_DATA->points)

void tetris_init_seed(unsigned int seed);
int custom_rand(void);
Tetrimino tetris_get_next_piece(void);
void delay_ms(unsigned int ms);
int check_collision_ext(int r, int c, int w, int h);
void draw_shape_ext(int r, int c, int w, int h, int is_erase);
void rotate_grid(int w, int h);
void clear_full_lines(void);
unsigned int dec_to_hex_display(unsigned int dec);
unsigned int get_delay_from_points(unsigned int p);


void __attribute__((interrupt("machine"), section(".trap_vector"))) isr_handler() {
    unsigned int cause, mepc;
    asm volatile ("csrr %0, mcause" : "=r"(cause));
    asm volatile ("csrr %0, mepc" : "=r"(mepc));

    if (cause <= 0x00000008) {
        //*hexData = mepc | cause<<12;
        asm volatile ("csrw mepc, %0" :: "r"(mepc+4));
    }
    else if (cause == 0x80000010) {
        key = *ps2Data;
        if (key == 0x129) {
            start = 1;
        }
        else if (key == 0x36B) { // LEVO
			dir = 1;
		}
		else if (key == 0x374) { // DESNO
			dir = 2;
		}
		else if (key == 0x372) { // DOLE
			dir = 5;
		}
		else if (key == 0x375) { // GORE (Rotacija)
			dir = 6;
		}
    }
    else if (cause == 0x80000011) {
        *vgaData = *(matrix + matrixPrintIndex);
        *vgaAddr = matrixPrintIndex++;
        if (matrixPrintIndex >= 200) {
            matrixPrintIndex = 0;
            *vgaCont = 0x1;
        }
    }
	
}

void main() {
	int firstGame = 1, faza = 0;
startGame:
    // Onemogući prekide dok traje ponovna inicijalizacija
    asm volatile ("csrc mstatus, %0" :: "r"(MSTATUS_MIE));

    unsigned int i;
    g_rng_state = 2;
    start = 0;
    index = 7;
    matrixPrintIndex = 0;
    row = 0;
    col = 4;
    dir = 0;
    

    // Inicijalizacija nizova sa bojama i oblicima
    *(colors+0) = 0x0FF; *(colors+1) = 0x00F; *(colors+2) = 0xF70;
    *(colors+3) = 0xFF0; *(colors+4) = 0x0F0; *(colors+5) = 0xA0F; *(colors+6) = 0xF00;
    
    *(shapes+0) = 0xf0; *(shapes+1) = 0xe2; *(shapes+2) = 0xe8;
    *(shapes+3) = 0x66; *(shapes+4) = 0x6c; *(shapes+5) = 0x72; *(shapes+6) = 0x63;
    
    // Inicijalizacija matrice na 0
    for(i = 0; i < 200; i++) *(matrix + i) = 0;

    asm volatile ("csrw mtvec, %0" :: "r"((void *)0x3000));
    asm volatile ("csrs mie, %0" :: "r"(0x800));
    
    *ps2Cont = 0x3;
    *vgaCont = 0x1;
    
    // Omogući prekide za pritisak tastera za start
    asm volatile ("csrs mstatus, %0" :: "r"(MSTATUS_MIE));
	while (!start) {
        if (!firstGame) {
            *hexData = dec_to_hex_display(points);
            delay_ms(400);
            if (start) break;
            
            *hexData = 0xDEAD;
            delay_ms(400);
        }
    }
    
    // Započinjanje nove igre
    firstGame = 0;
    points = 0;
    *hexData = points;
    
    asm volatile ("csrr %0, 0xb00" : "=r"(g_rng_state));
    
    Tetrimino next_piece = tetris_get_next_piece();

    while (1) {
        Tetrimino current_piece = next_piece;
        int current_shape_bits = shapeNext; 
        
        next_piece = tetris_get_next_piece();

        for(i = 0; i < 16; i++){
            current_grid[i] = 0;
        }
        
        int cur_w = 4;
        int cur_h = 2;

        // Inicijalizacija iz bitmaske (početno 4x2)
        for (i = 0; i < 8; i++) {
            if ((current_shape_bits) & 1) {
                current_grid[i] = current_piece.color;
            } else {
                current_grid[i] = 0;
            }    
            current_shape_bits >>= 1;
        }

        *vgaColor = next_piece.color;
        *vgaShape = shapeNext;
        *vgaCont  = 0x3;
        
        int cur_row = -2;
        int cur_col = 3;

		
        // Provera da li nova figura ulazi na popunjeno mesto (KRAJ IGRE)
        int hit_bottom = 0;

        while (!hit_bottom) {
            
            asm volatile ("csrc mstatus, %0" :: "r"(MSTATUS_MIE));
            draw_shape_ext(cur_row, cur_col, cur_w, cur_h, 1); // Privremeno brisanje

            int req_dir = dir;
            dir = 0; // Resetuj komandu

            if (req_dir == 1) { // LEVO
                if (!check_collision_ext(cur_row, cur_col - 1, cur_w, cur_h)) {
                    cur_col--;
                }
            } 
            else if (req_dir == 2) { // DESNO
                if (!check_collision_ext(cur_row, cur_col + 1, cur_w, cur_h)) {
                    cur_col++;
                }
            } 
            else if (req_dir == 5) { // DOLE
                if (!check_collision_ext(cur_row + 1, cur_col, cur_w, cur_h)) {
                    cur_row++;
                    points++;
					*hexData = dec_to_hex_display(points);
                }
            } 
            else if (req_dir == 6) { // ROTACIJA
                for(i = 0; i < 16; i++){
                    rotated_grid[i] = 0;
                    current_grid_temp[i] = 0;
                }
                int next_w = cur_h;
                int next_h = cur_w;

                rotate_grid(cur_w, cur_h);

                for (int k = 0; k < 16; k++){
                    current_grid_temp[k] = current_grid[k];
                    current_grid[k] = rotated_grid[k];
                }
                if (!check_collision_ext(cur_row, cur_col, next_w, next_h)) {
                    cur_w = next_w;
                    cur_h = next_h;
                }
                else {
                    for (int k = 0; k < 16; k++) current_grid[k] = current_grid_temp[k];
                }
            }

            // REDOVNO SPUŠTANJE ZA JEDAN RED
            if (check_collision_ext(cur_row + 1, cur_col, cur_w, cur_h)) {
                hit_bottom = 1;
            } else {
                cur_row++;
            }

            draw_shape_ext(cur_row, cur_col, cur_w, cur_h, 0); // Iscrtavanje na novoj poziciji
            asm volatile ("csrs mstatus, %0" :: "r"(MSTATUS_MIE));

            *vgaCont = 0x3;
            //*hexData = points;
            delay_ms(get_delay_from_points(points)); 
			
        }
		
		for(i=0;i<10;i++){
			if(matrix[i] != 0){
				*hexData = 0xDEAD; // Signalizacija Game Over stanja
				//delay_ms(1500);    // Pauza da igrač vidi kraj
				goto startGame;
			}
			
		}

        // Nakon zaklučavanja figure na dnu, proveravamo i brišemo pune redove
        asm volatile ("csrc mstatus, %0" :: "r"(MSTATUS_MIE));
        clear_full_lines();
        asm volatile ("csrs mstatus, %0" :: "r"(MSTATUS_MIE));
    }

    HALT;
}
// Funkcija za detekciju i brisanje punih redova
void clear_full_lines(void) {
    for (int r = 19; r >= 0; r--) {
        int full = 1;
        for (int c = 0; c < 10; c++) {
            if (*(matrix + r * 10 + c) == 0) {
                full = 0;
                break;
            }
        }
        
        // Ako je red pun, pomeramo sve iznad njega za jedno mesto dole
        if (full) {
            for (int shift_r = r; shift_r > 0; shift_r--) {
                for (int c = 0; c < 10; c++) {
                    *(matrix + shift_r * 10 + c) = *(matrix + (shift_r - 1) * 10 + c);
                }
            }
            // Najviši red praznimo
            for (int c = 0; c < 10; c++) {
                *(matrix + c) = 0;
            }
            // Proveravamo isti indeks opet jer je nova linija pala na ovo mesto
            r++; 
            points += 10; 
			*hexData = dec_to_hex_display(points);
        }
    }
}

void delay_ms(unsigned int ms) {
    unsigned long cycles_per_ms = (unsigned long)((50000000UL / 10000UL) / 5.6);
    unsigned int i;
    volatile unsigned long j;

    for (i = 0; i < ms; i++) {
        for (j = 0; j < cycles_per_ms; j++) {}
    }
}

void tetris_init_seed(unsigned int seed) {
    g_rng_state = seed;
}

int custom_rand(void) {
    g_rng_state = g_rng_state * 1103515245 + 12345;
    return (g_rng_state >> 16) & 0x7FFF;
}

Tetrimino tetris_get_next_piece(void) {
    int i;
    if (index >= 7) {
        for (i = 0; i < 7; i++) bag[i] = i;

        for (i = 6; i > 0; i--) {
            int j = custom_rand() % (i + 1);
            int temp = bag[i];
            bag[i] = bag[j];
            bag[j] = temp;
        }
        index = 0;
    }
    Tetrimino piece;
    piece.type = (int)bag[index];
    piece.color = colors[piece.type];
    shapeNext = shapes[piece.type];

    index++;
    return piece;
}

int check_collision_ext(int r, int c, int w, int h) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int idx = i * w + j;
            if (idx < 16 && current_grid[idx] != 0) { // Direktno čitanje current_grid
                int map_r = r + i;
                int map_c = c + j;

                if (map_c < 0 || map_c >= 10 || map_r >= 20) return 1;
                if (map_r >= 0 && *(matrix + map_r * 10 + map_c) != 0) return 1;
            }
        }
    }
    return 0;
}

void draw_shape_ext(int r, int c, int w, int h, int is_erase) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int val = current_grid[i * w + j];
            if (val != 0) {
                int map_r = r + i;
                int map_c = c + j;
                if (map_r >= 0 && map_r < 20 && map_c >= 0 && map_c < 10) {
                    *(matrix + map_r * 10 + map_c) = is_erase ? 0 : val;
                }
            }
        }
    }
}

void rotate_grid(int w, int h) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            rotated_grid[j * h + (h - 1 - i)] = current_grid[i * w + j];
        }
    }
}

unsigned int dec_to_hex_display(unsigned int dec) {
    unsigned int d3 = (dec / 1000) % 10;
    unsigned int d2 = (dec / 100) % 10;
    unsigned int d1 = (dec / 10) % 10;
    unsigned int d0 = dec % 10;

    return (d3 << 12) | (d2 << 8) | (d1 << 4) | d0;
}

// Vraća trajanje pauze u ms u zavisnosti od sakupljenih poena
unsigned int get_delay_from_points(unsigned int p) {
    if (p < 50)   return 350; // Početna brzina (Lako)
    if (p < 150)  return 300;
    if (p < 300)  return 250;
    if (p < 500)  return 190;
    if (p < 800)  return 140;
    if (p < 1200) return 100;
    if (p < 1700) return 70;
    return 40;                // Maksimalna brzina za 1700+ poena
}