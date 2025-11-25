#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzles.h"
#include <fat.h>

// ============================================================================
// 🎨 CORES & CONFIG
// ============================================================================
#define COLOR_BG        (RGB15(28, 29, 31) | BIT(15))
#define COLOR_GRID      (RGB15(22, 23, 25) | BIT(15))
#define COLOR_CLUE_TXT  (RGB15(5,  8,  12) | BIT(15))
#define COLOR_CLUE_DONE (RGB15(20, 20, 20) | BIT(15))
#define COLOR_FILLED    (RGB15(31, 10, 5)  | BIT(15))
#define COLOR_MARKER    (RGB15(15, 15, 15) | BIT(15))
#define COLOR_CURSOR    (RGB15(0,  25, 31) | BIT(15))
#define COLOR_WIN_BG    (RGB15(22, 31, 22) | BIT(15))

// --- SISTEMA DE PARTÍCULAS ---
#define MAX_PARTICLES 150
typedef struct {
    float x, y;
    float vx, vy;
    int life;
    u16 color;
    bool active;
} Particle;

Particle particles[MAX_PARTICLES];
bool fireworksActive = false;

// --- PROTÓTIPOS ---
void triggerExplosion(); 
void updateAndDrawFireworks(); 
void saveGame(); 
void loadGame();

#define CELL_SIZE 10
#define GRID_COLS 15
#define GRID_ROWS 15
#define MARGIN_LEFT 60 
#define MARGIN_TOP  40
#define MAX_CLUES 8

// --- SAVE DATA ---
typedef struct {
    unsigned int score;
    bool solved[1000]; 
} SaveData;

SaveData saveData;
bool fatReady = false; // <--- NOVA FLAG DE SEGURANÇA

u16* videoBuffer;
int currentPuzzleIndex = 0;
int playerGrid[GRID_ROWS][GRID_COLS];
bool gameWon = false;

bool rowDone[GRID_ROWS];
bool colDone[GRID_COLS];

typedef struct { int count; int values[MAX_CLUES]; } LineClues;
LineClues rowClues[GRID_ROWS];
LineClues colClues[GRID_COLS];

// Variáveis de Input
bool isDragging = false;
int dragType = 0; 

// Aleatoriedade
int puzzleBag[1000]; 
int bagIndex = 0;   

// ============================================================================
// 🔊 GESTOR DE SOM
// ============================================================================
int soundChannel = -1;
int soundTimer = 0;   

void playSound(int type) {
    if (soundChannel != -1) soundKill(soundChannel);

    if (type == 0) { // PINTAR
        soundChannel = soundPlayPSG(DutyCycle_50, 400, 60, 64);
        soundTimer = 4;
    } else if (type == 1) { // MARCAR
        soundChannel = soundPlayPSG(DutyCycle_12, 2000, 50, 64);
        soundTimer = 3;
    } else if (type == 2) { // APAGAR
        soundChannel = soundPlayNoise(1500, 40, 64);
        soundTimer = 5;
    } else if (type == 3) { // VITÓRIA
        soundChannel = soundPlayPSG(DutyCycle_50, 880, 80, 64);
        soundTimer = 30;
    }
}

void updateSound() {
    if (soundTimer > 0) {
        soundTimer--;
        if (soundTimer == 0 && soundChannel != -1) {
            soundKill(soundChannel);
            soundChannel = -1;
        }
    }
}

// ============================================================================
// 🧠 LÓGICA DO JOGO
// ============================================================================

void calculateTargetClues() {
    const Puzzle* p = &allPuzzles[currentPuzzleIndex];
    for(int r=0; r<GRID_ROWS; r++) {
        int idx = 0, count = 0;
        for(int c=0; c<GRID_COLS; c++) {
            if(p->grid[r][c] == 1) count++;
            else if(count > 0) { if(idx<MAX_CLUES) rowClues[r].values[idx++] = count; count = 0; }
        }
        if(count > 0 && idx<MAX_CLUES) rowClues[r].values[idx++] = count;
        rowClues[r].count = idx;
    }
    for(int c=0; c<GRID_COLS; c++) {
        int idx = 0, count = 0;
        for(int r=0; r<GRID_ROWS; r++) {
            if(p->grid[r][c] == 1) count++;
            else if(count > 0) { if(idx<MAX_CLUES) colClues[c].values[idx++] = count; count = 0; }
        }
        if(count > 0 && idx<MAX_CLUES) colClues[c].values[idx++] = count;
        colClues[c].count = idx;
    }
}

bool checkLineMatch(int index, bool isRow) {
    int currentClues[MAX_CLUES];
    int idx = 0, count = 0;
    for(int i=0; i<15; i++) {
        int cell = isRow ? playerGrid[index][i] : playerGrid[i][index];
        if(cell == 1) count++;
        else if(count > 0) { if(idx<MAX_CLUES) currentClues[idx++] = count; count = 0; }
    }
    if(count > 0 && idx<MAX_CLUES) currentClues[idx++] = count;

    LineClues* target = isRow ? &rowClues[index] : &colClues[index];
    if (idx != target->count) return false;
    for(int k=0; k<idx; k++) if (currentClues[k] != target->values[k]) return false;
    return true;
}

void updateClueStates() {
    for(int r=0; r<GRID_ROWS; r++) rowDone[r] = checkLineMatch(r, true);
    for(int c=0; c<GRID_COLS; c++) colDone[c] = checkLineMatch(c, false);
}

void resetGame() {
    for(int i=0; i<GRID_ROWS; i++) for(int j=0; j<GRID_COLS; j++) playerGrid[i][j] = 0;
    gameWon = false;
    calculateTargetClues();
    updateClueStates();
}

void checkWin() {
    if (gameWon) return;
    updateClueStates();
    const Puzzle* p = &allPuzzles[currentPuzzleIndex];
    bool match = true;
    int pixelCount = 0;

    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            if (p->grid[r][c] == 1) pixelCount++;
            if ((p->grid[r][c] == 1 && playerGrid[r][c] != 1) ||
                (p->grid[r][c] == 0 && playerGrid[r][c] == 1)) {
                match = false; 
            }
        }
    }
    
    if (!match) return;

    gameWon = true;
    playSound(3);
    triggerExplosion();

    if (!saveData.solved[currentPuzzleIndex]) {
        saveData.solved[currentPuzzleIndex] = true;
        int pointsEarned = 50 + (pixelCount * 2);
        saveData.score += pointsEarned;
        saveGame();
    }
}

// ============================================================================
// 🖌️ MOTOR GRÁFICO
// ============================================================================
void plot(int x, int y, u16 color) {
    if (x >= 0 && x < 256 && y >= 0 && y < 192) videoBuffer[y * 256 + x] = color;
}

void drawRect(int x, int y, int w, int h, u16 color) {
    for(int i=0; i<h; i++) for(int j=0; j<w; j++) plot(x+j, y+i, color);
}

const u8 miniFont[10][3] = {
    {0x1F, 0x11, 0x1F}, {0x00, 0x1F, 0x00}, {0x1D, 0x15, 0x17}, {0x15, 0x15, 0x1F},
    {0x07, 0x04, 0x1F}, {0x17, 0x15, 0x1D}, {0x1F, 0x15, 0x1D}, {0x01, 0x01, 0x1F},
    {0x1F, 0x15, 0x1F}, {0x17, 0x15, 0x1F}
};

void drawMiniNum(int x, int y, int num, u16 color) {
    if (num > 9) { drawMiniNum(x - 4, y, num / 10, color); num %= 10; }
    for (int col = 0; col < 3; col++) {
        u8 colData = miniFont[num][col];
        for (int row = 0; row < 5; row++) if ((colData >> row) & 1) plot(x + col, y + row, color);
    }
}

void drawX(int x, int y, int s, u16 c) {
    for(int i=0; i<s; i++) { plot(x+i, y+i, c); plot(x+s-1-i, y+i, c); }
}

void drawXMarker(int x, int y) { drawX(x+2, y+2, CELL_SIZE-5, COLOR_MARKER); }

void drawCursor(int r, int c) {
    int x = MARGIN_LEFT + (c * CELL_SIZE);
    int y = MARGIN_TOP + (r * CELL_SIZE);
    drawRect(x, y, CELL_SIZE, 1, COLOR_CURSOR);
    drawRect(x, y+CELL_SIZE-1, CELL_SIZE, 1, COLOR_CURSOR);
    drawRect(x, y, 1, CELL_SIZE, COLOR_CURSOR);
    drawRect(x+CELL_SIZE-1, y, 1, CELL_SIZE, COLOR_CURSOR);
}

void renderGame(int cursorR, int cursorC) {
    u16 bg = gameWon ? COLOR_WIN_BG : COLOR_BG;
    for(int i=0; i<256*192; i++) videoBuffer[i] = bg;

    for(int c=0; c<GRID_COLS; c++) {
        u16 txtColor = colDone[c] ? COLOR_CLUE_DONE : COLOR_CLUE_TXT;
        int count = colClues[c].count;
        int x = MARGIN_LEFT + (c * CELL_SIZE) + 3;
        int yBase = MARGIN_TOP - 2;
        for(int i=count-1; i>=0; i--) drawMiniNum(x, yBase - ((count-1-i)*7) - 6, colClues[c].values[i], txtColor);
    }
    for(int r=0; r<GRID_ROWS; r++) {
        u16 txtColor = rowDone[r] ? COLOR_CLUE_DONE : COLOR_CLUE_TXT;
        int count = rowClues[r].count;
        int y = MARGIN_TOP + (r * CELL_SIZE) + 3;
        int xBase = MARGIN_LEFT - 2;
        for(int i=count-1; i>=0; i--) {
            int val = rowClues[r].values[i];
            int offset = (val > 9) ? 8 : 4;
            drawMiniNum(xBase - offset, y, val, txtColor);
            xBase -= (offset + 3);
        }
    }

    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            int px = MARGIN_LEFT + (c * CELL_SIZE);
            int py = MARGIN_TOP + (r * CELL_SIZE);
            u16 color = COLOR_BG; 
            if (playerGrid[r][c] == 1) color = COLOR_FILLED;
            
            drawRect(px, py, CELL_SIZE-1, CELL_SIZE-1, color);
            drawRect(px+CELL_SIZE-1, py, 1, CELL_SIZE, COLOR_GRID);
            drawRect(px, py+CELL_SIZE-1, CELL_SIZE, 1, COLOR_GRID);

            if (playerGrid[r][c] == 2) drawXMarker(px, py);
        }
    }
    drawRect(MARGIN_LEFT-1, MARGIN_TOP-1, 1, (GRID_ROWS*CELL_SIZE)+1, COLOR_CLUE_TXT);
    drawRect(MARGIN_LEFT-1, MARGIN_TOP-1, (GRID_COLS*CELL_SIZE)+1, 1, COLOR_CLUE_TXT);

    if (!gameWon && cursorR >= 0) drawCursor(cursorR, cursorC);
}

void shuffleBag() {
    for (int i = 0; i < PUZZLE_COUNT; i++) puzzleBag[i] = i;
    for (int i = PUZZLE_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = puzzleBag[i];
        puzzleBag[i] = puzzleBag[j];
        puzzleBag[j] = temp;
    }
    bagIndex = 0;
}

int getNextPuzzleID() {
    if (bagIndex >= PUZZLE_COUNT) shuffleBag();
    return puzzleBag[bagIndex++];
}

void triggerExplosion() {
    fireworksActive = true;
    for(int i=0; i<MAX_PARTICLES; i++) {
        particles[i].active = true;
        particles[i].x = 128;
        particles[i].y = 80;
        particles[i].life = 60 + (rand() % 60);
        int r = rand() % 31; int g = rand() % 31; int b = rand() % 31;
        particles[i].color = RGB15(r, g, b) | BIT(15);
        particles[i].vx = ((rand() % 100) / 20.0) - 2.5; 
        particles[i].vy = ((rand() % 100) / 20.0) - 3.5;
    }
}

void updateAndDrawFireworks() {
    int activeCount = 0;
    for(int i=0; i<MAX_PARTICLES; i++) {
        if(!particles[i].active) continue;
        activeCount++;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.08;
        particles[i].life--;
        if(particles[i].life <= 0 || particles[i].y > 192 || particles[i].x < 0 || particles[i].x > 256) {
            particles[i].active = false;
            continue;
        }
        int px = (int)particles[i].x; int py = (int)particles[i].y;
        plot(px, py, particles[i].color); plot(px+1, py, particles[i].color);
        plot(px, py+1, particles[i].color); plot(px+1, py+1, particles[i].color);
    }
    if(activeCount == 0) fireworksActive = false;
}

// --- SAVE / LOAD SEGUROS ---
void saveGame() {
    if (!fatReady) return; // Se não houver cartão, ignora
    FILE* file = fopen("pikranji.sav", "wb");
    if (file) {
        fwrite(&saveData, sizeof(SaveData), 1, file);
        fclose(file);
    }
}

void loadGame() {
    saveData.score = 0;
    for(int i=0; i<1000; i++) saveData.solved[i] = false;

    if (!fatReady) return; // Se não houver cartão, ignora

    FILE* file = fopen("pikranji.sav", "rb");
    if (file) {
        fread(&saveData, sizeof(SaveData), 1, file);
        fclose(file);
    } 
}

// ============================================================================
// MAIN LOOP
// ============================================================================
int main(void) {
    irqEnable(IRQ_VBLANK);
    soundEnable(); 

    videoSetMode(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    consoleInit(0, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    int bg3 = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    videoBuffer = bgGetGfxPtr(bg3);

    // 4. Inicializar FAT com segurança
    fatReady = fatInitDefault(); 
    
    // 5. Carregar e inicializar
    loadGame(); 
    srand(time(NULL)); 
    shuffleBag();
    currentPuzzleIndex = getNextPuzzleID();
    resetGame();
    
    int lastR = -1, lastC = -1;
    bool forceRender = true;

    iprintf("\x1b[4;2HControlos (Stylus +):");
    iprintf("\x1b[6;2H[Seta BAIXO] Pintar");
    iprintf("\x1b[7;2H[Seta CIMA]  Marcar (X)");
    iprintf("\x1b[9;2H[Select]     Prox. Puzzle");
    iprintf("\x1b[10;2H[Start]      Reiniciar");

    while(1) {
        swiIntrWait(1, IRQ_VBLANK);
        updateSound();
        
        scanKeys();
        int held = keysHeld();
        int down = keysDown();

        if (down & KEY_START) { resetGame(); forceRender = true; }
        if (down & KEY_SELECT) { currentPuzzleIndex = getNextPuzzleID(); resetGame(); forceRender = true; }

        int currR = -1, currC = -1;
        if (held & KEY_TOUCH) {
            touchPosition touch;
            touchRead(&touch);
            int c = (touch.px - MARGIN_LEFT) / CELL_SIZE;
            int r = (touch.py - MARGIN_TOP) / CELL_SIZE;

            if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS) {
                currR = r; currC = c;
                if (!gameWon) {
                    if (down & KEY_TOUCH) {
                        isDragging = true;
                        if (held & KEY_UP) dragType = (playerGrid[r][c] == 2) ? 2 : 3; 
                        else if (held & KEY_DOWN) dragType = (playerGrid[r][c] == 1) ? 2 : 1; 
                        else dragType = 0; 
                    }
                    if (isDragging && dragType != 0) {
                        int newState = -1;
                        if (dragType == 1) newState = 1; 
                        else if (dragType == 2) newState = 0; 
                        else if (dragType == 3) newState = 2; 

                        if (playerGrid[r][c] != newState) {
                            playerGrid[r][c] = newState;
                            if (newState == 1) playSound(0);
                            else if (newState == 2) playSound(1);
                            else if (newState == 0) playSound(2);
                            checkWin(); 
                            forceRender = true;
                        }
                    }
                }
            }
        } else {
            isDragging = false;
            dragType = 0;
        }

        if (forceRender || currR != lastR || currC != lastC || fireworksActive) {
            const Puzzle* p = &allPuzzles[currentPuzzleIndex];
            
            iprintf("\x1b[2;2H-- PIKRANJI DS --");
            iprintf("\x1b[2;20HScore: %d     ", saveData.score); 
            
            iprintf("\x1b[14;2HPuzzle: %d / %d   ", currentPuzzleIndex + 1, PUZZLE_COUNT);
            
            if (fatReady && saveData.solved[currentPuzzleIndex]) {
                 iprintf("\x1b[14;20H\x1b[33m[RESOLVIDO]\x1b[39m");
            } else {
                 iprintf("\x1b[14;20H           ");
            }

            iprintf("\x1b[16;2HSignificado:                                ");
            iprintf("\x1b[16;2HSignificado: \x1b[32m%s\x1b[39m", p->meaning);
            
            if (gameWon) iprintf("\x1b[18;2H** COMPLETO! ** ");
            else iprintf("\x1b[18;2H                  ");

            renderGame(currR, currC);
            if(fireworksActive) updateAndDrawFireworks();

            lastR = currR; lastC = currC;
            forceRender = false;
        }
    }
    return 0;
}
