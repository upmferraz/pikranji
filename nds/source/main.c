#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzles.h"
#include <fat.h>

// ============================================================================
// 🎵 CONFIGURAÇÃO DE ÁUDIO & ASSETS
// ============================================================================
#define SAMPLE_RATE 8000
#define DATA_FORMAT SoundFormat_8Bit

// Dados binários (Assembly Injection)
extern const u8 music_bin[];
extern const u32 music_bin_size;

extern const u8 sea1_bin[];
extern const u8 sea2_bin[];
extern const u8 sea3_bin[];

// ============================================================================
// 🌊 SISTEMA DE VÍDEO (TOP & BOTTOM)
// ============================================================================
u16* topScreenBuf;    // Ecrã de Cima (Imagens)
u16* bottomScreenBuf; // Ecrã de Baixo (Jogo)

const u8* bgImages[] = { sea1_bin, sea2_bin, sea3_bin };
#define NUM_BG_IMAGES 3

int currentBgIndex = 0;
int bgTimer = 0;
#define BG_SWITCH_TIME (60 * 10) 

void switchBackground() {
    currentBgIndex = (currentBgIndex + 1) % NUM_BG_IMAGES;
    dmaCopy(bgImages[currentBgIndex], topScreenBuf, 256*192*2);
}

// ============================================================================
// 🌍 LOCALIZAÇÃO & TEXTOS
// ============================================================================
#define LANG_PT 0
#define LANG_EN 1

const char* UI_TEXT[2][6] = {
    {"Score", "Desbloq", "RESOLVIDO", "BATOTA", "Significado", "COMPLETO!"},
    {"Score", "Unlock",  "SOLVED",    "CHEATED", "Meaning",     "COMPLETE!"}
};

// ============================================================================
// 🎨 CORES & TEMAS
// ============================================================================
typedef struct { u16 bg; u16 grid; u16 filled; u16 cursor; u16 clue_txt; u16 clue_done; } ColorTheme;

ColorTheme themes[] = {
    { (RGB15(28, 29, 31) | BIT(15)), (RGB15(22, 23, 25) | BIT(15)), (RGB15(31, 10, 5) | BIT(15)), (RGB15(0, 25, 31) | BIT(15)), (RGB15(5, 8, 12) | BIT(15)), (RGB15(20, 20, 20) | BIT(15)) },
    { (RGB15(26, 30, 26) | BIT(15)), (RGB15(20, 24, 20) | BIT(15)), (RGB15(10, 25, 10) | BIT(15)), (RGB15(31, 20, 10) | BIT(15)), (RGB15(5, 15, 5) | BIT(15)), (RGB15(18, 22, 18) | BIT(15)) },
    { (RGB15(25, 28, 30) | BIT(15)), (RGB15(20, 23, 25) | BIT(15)), (RGB15(5, 20, 31) | BIT(15)), (RGB15(31, 25, 10) | BIT(15)), (RGB15(0, 10, 20) | BIT(15)), (RGB15(18, 20, 22) | BIT(15)) },
    { (RGB15(30, 28, 26) | BIT(15)), (RGB15(25, 23, 21) | BIT(15)), (RGB15(25, 12, 5) | BIT(15)), (RGB15(5, 20, 15) | BIT(15)), (RGB15(15, 8, 5) | BIT(15)), (RGB15(24, 22, 20) | BIT(15)) },
    { (RGB15(29, 27, 30) | BIT(15)), (RGB15(24, 22, 25) | BIT(15)), (RGB15(20, 10, 25) | BIT(15)), (RGB15(10, 31, 20) | BIT(15)), (RGB15(12, 5, 15) | BIT(15)), (RGB15(22, 20, 22) | BIT(15)) }
};

#define NUM_THEMES 5
int currentThemeIdx = 0;
#define COLOR_MARKER    (RGB15(15, 15, 15) | BIT(15))
#define COLOR_WIN_BG    (RGB15(22, 31, 22) | BIT(15))
#define COLOR_BTN       (RGB15(10, 10, 10) | BIT(15))
#define COLOR_BTN_TXT   (RGB15(25, 25, 25) | BIT(15))

#define MAX_PARTICLES 150
typedef struct { float x, y, vx, vy; int life; u16 color; bool active; } Particle;
Particle particles[MAX_PARTICLES];
bool fireworksActive = false;

// --- PROTÓTIPOS ---
void triggerExplosion(); void updateAndDrawFireworks(); void saveGame(); void loadGame();
void useHint(); void solvePuzzle(); void initPuzzleSystem(); void refreshStaticText(int lang);

#define CELL_SIZE 10
#define GRID_COLS 15
#define GRID_ROWS 15
#define MARGIN_LEFT 60 
#define MARGIN_TOP  35 
#define MAX_CLUES 8

typedef struct { int score; int unlockedLimit; int solvedCount; bool solved[1000]; int language; } SaveData;
SaveData saveData;
bool fatReady = false;

int currentPuzzleIndex = 0; int playerGrid[GRID_ROWS][GRID_COLS];
bool gameWon = false; bool cheated = false;
bool rowDone[GRID_ROWS]; bool colDone[GRID_COLS];
typedef struct { int count; int values[MAX_CLUES]; } LineClues;
LineClues rowClues[GRID_ROWS]; LineClues colClues[GRID_COLS];
typedef struct { int id; int complexity; } PuzzleEntry;
PuzzleEntry sortedPuzzles[1000]; 

bool isDragging = false; int dragType = 0; 
int puzzleBag[1000]; int bagIndex = 0;   

#define BTN_W 36
#define BTN_H 25
#define BTN_HINT_X 216
#define BTN_HINT_Y 60
#define BTN_SOLVE_X 216
#define BTN_SOLVE_Y 100

// ============================================================================
// 🔊 GESTOR DE SOM (SFX)
// ============================================================================
int soundChannel = -1; int soundTimer = 0;   
void playSound(int type) {
    if (soundChannel != -1) soundKill(soundChannel);
    if (type == 0) { soundChannel = soundPlayPSG(DutyCycle_50, 400, 60, 64); soundTimer = 4; }
    else if (type == 1) { soundChannel = soundPlayPSG(DutyCycle_12, 2000, 50, 64); soundTimer = 3; }
    else if (type == 2) { soundChannel = soundPlayNoise(1500, 40, 64); soundTimer = 5; }
    else if (type == 3) { soundChannel = soundPlayPSG(DutyCycle_50, 880, 80, 64); soundTimer = 30; }
    else if (type == 4) { soundChannel = soundPlayPSG(DutyCycle_25, 150, 60, 64); soundTimer = 10; }
}
void updateSound() {
    if (soundTimer > 0) { soundTimer--; if (soundTimer == 0 && soundChannel != -1) { soundKill(soundChannel); soundChannel = -1; } }
}

// ============================================================================
// LÓGICA DO JOGO (Standard)
// ============================================================================
int comparePuzzles(const void *a, const void *b) {
    PuzzleEntry *pa = (PuzzleEntry *)a; PuzzleEntry *pb = (PuzzleEntry *)b;
    return (pa->complexity - pb->complexity);
}
void initPuzzleSystem() {
    for (int i = 0; i < PUZZLE_COUNT; i++) {
        sortedPuzzles[i].id = i; int pixels = 0;
        for (int r = 0; r < 15; r++) for (int c = 0; c < 15; c++) if (allPuzzles[i].grid[r][c] == 1) pixels++;
        sortedPuzzles[i].complexity = pixels;
    }
    qsort(sortedPuzzles, PUZZLE_COUNT, sizeof(PuzzleEntry), comparePuzzles);
}
void calculateTargetClues() {
    const Puzzle* p = &allPuzzles[currentPuzzleIndex];
    for(int r=0; r<GRID_ROWS; r++) {
        int idx = 0, count = 0;
        for(int c=0; c<GRID_COLS; c++) {
            if(p->grid[r][c] == 1) count++; else if(count > 0) { if(idx<MAX_CLUES) rowClues[r].values[idx++] = count; count = 0; }
        }
        if(count > 0 && idx<MAX_CLUES) rowClues[r].values[idx++] = count; rowClues[r].count = idx;
    }
    for(int c=0; c<GRID_COLS; c++) {
        int idx = 0, count = 0;
        for(int r=0; r<GRID_ROWS; r++) {
            if(p->grid[r][c] == 1) count++; else if(count > 0) { if(idx<MAX_CLUES) colClues[c].values[idx++] = count; count = 0; }
        }
        if(count > 0 && idx<MAX_CLUES) colClues[c].values[idx++] = count; colClues[c].count = idx;
    }
}
bool checkLineMatch(int index, bool isRow) {
    int currentClues[MAX_CLUES]; int idx = 0, count = 0;
    for(int i=0; i<15; i++) {
        int cell = isRow ? playerGrid[index][i] : playerGrid[i][index];
        if(cell == 1) count++; else if(count > 0) { if(idx<MAX_CLUES) currentClues[idx++] = count; count = 0; }
    }
    if(count > 0 && idx<MAX_CLUES) currentClues[idx++] = count;
    LineClues* target = isRow ? &rowClues[index] : &colClues[index];
    if (idx != target->count) return false;
    for(int k=0; k<idx; k++) if (currentClues[k] != target->values[k]) return false;
    return true;
}
void updateClueStates() { for(int r=0; r<GRID_ROWS; r++) rowDone[r] = checkLineMatch(r, true); for(int c=0; c<GRID_COLS; c++) colDone[c] = checkLineMatch(c, false); }
void shuffleBag() {
    int limit = saveData.unlockedLimit; if (limit > PUZZLE_COUNT) limit = PUZZLE_COUNT;
    for (int i = 0; i < limit; i++) puzzleBag[i] = sortedPuzzles[i].id;
    for (int i = limit - 1; i > 0; i--) { int j = rand() % (i + 1); int temp = puzzleBag[i]; puzzleBag[i] = puzzleBag[j]; puzzleBag[j] = temp; }
    bagIndex = 0;
}
int getNextPuzzleID() {
    int limit = saveData.unlockedLimit; if (limit > PUZZLE_COUNT) limit = PUZZLE_COUNT;
    if (bagIndex >= limit) shuffleBag(); return puzzleBag[bagIndex++];
}
void resetGame() {
    for(int i=0; i<GRID_ROWS; i++) for(int j=0; j<GRID_COLS; j++) playerGrid[i][j] = 0;
    gameWon = false; cheated = false; currentThemeIdx = rand() % NUM_THEMES;
    calculateTargetClues(); updateClueStates();
}
void checkWin() {
    if (gameWon) return; updateClueStates(); const Puzzle* p = &allPuzzles[currentPuzzleIndex];
    bool match = true; int pixelCount = 0;
    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            if (p->grid[r][c] == 1) pixelCount++;
            if ((p->grid[r][c] == 1 && playerGrid[r][c] != 1) || (p->grid[r][c] == 0 && playerGrid[r][c] == 1)) match = false; 
        }
    }
    if (!match) return;
    gameWon = true; playSound(3); triggerExplosion();
    if (!saveData.solved[currentPuzzleIndex] && !cheated) {
        saveData.solved[currentPuzzleIndex] = true; saveData.solvedCount++; saveData.score += 50 + (pixelCount * 2);
        if (saveData.solvedCount >= (saveData.unlockedLimit - 5)) { if (saveData.unlockedLimit < PUZZLE_COUNT) { saveData.unlockedLimit += 10; bagIndex = 9999; } }
        saveGame();
    }
}
void useHint() {
    if (gameWon) return; if (saveData.score > 0) { saveData.score--; saveGame(); } const Puzzle* p = &allPuzzles[currentPuzzleIndex];
    int attempts = 0;
    while(attempts < 100) {
        int r = rand() % GRID_ROWS; int c = rand() % GRID_COLS;
        if (rand() % 2 == 0) { if (!rowDone[r]) { for(int i=0; i<GRID_COLS; i++) playerGrid[r][i] = (p->grid[r][i] == 1) ? 1 : 2; playSound(1); checkWin(); return; } } 
        else { if (!colDone[c]) { for(int i=0; i<GRID_ROWS; i++) playerGrid[i][c] = (p->grid[i][c] == 1) ? 1 : 2; playSound(1); checkWin(); return; } }
        attempts++;
    }
    for(int r=0; r<GRID_ROWS; r++) { if (!rowDone[r]) { for(int i=0; i<GRID_COLS; i++) playerGrid[r][i] = (p->grid[r][i] == 1) ? 1 : 2; playSound(1); checkWin(); return; } }
}
void solvePuzzle() { if (gameWon) return; cheated = true; const Puzzle* p = &allPuzzles[currentPuzzleIndex]; for(int r=0; r<GRID_ROWS; r++) for(int c=0; c<GRID_COLS; c++) playerGrid[r][c] = (p->grid[r][c] == 1) ? 1 : 2; playSound(1); checkWin(); }

// 🖌️ MOTOR GRÁFICO (Bottom)
void plot(int x, int y, u16 color) { if (x >= 0 && x < 256 && y >= 0 && y < 192) bottomScreenBuf[y * 256 + x] = color; }
void drawRect(int x, int y, int w, int h, u16 color) { for(int i=0; i<h; i++) for(int j=0; j<w; j++) plot(x+j, y+i, color); }
const u8 miniFont[10][3] = { {0x1F, 0x11, 0x1F}, {0x00, 0x1F, 0x00}, {0x1D, 0x15, 0x17}, {0x15, 0x15, 0x1F}, {0x07, 0x04, 0x1F}, {0x17, 0x15, 0x1D}, {0x1F, 0x15, 0x1D}, {0x01, 0x01, 0x1F}, {0x1F, 0x15, 0x1F}, {0x17, 0x15, 0x1F} };
void drawMiniNum(int x, int y, int num, u16 color) {
    if (num > 9) { drawMiniNum(x - 4, y, num / 10, color); num %= 10; }
    for (int col = 0; col < 3; col++) { u8 colData = miniFont[num][col]; for (int row = 0; row < 5; row++) if ((colData >> row) & 1) plot(x + col, y + row, color); }
}
void drawButtons() {
    drawRect(BTN_HINT_X, BTN_HINT_Y, BTN_W, BTN_H, COLOR_BTN); drawRect(BTN_HINT_X+16, BTN_HINT_Y+6, 4, 2, COLOR_BTN_TXT); drawRect(BTN_HINT_X+20, BTN_HINT_Y+6, 2, 4, COLOR_BTN_TXT); drawRect(BTN_HINT_X+18, BTN_HINT_Y+10, 2, 2, COLOR_BTN_TXT); drawRect(BTN_HINT_X+18, BTN_HINT_Y+14, 2, 2, COLOR_BTN_TXT);
    drawRect(BTN_SOLVE_X, BTN_SOLVE_Y, BTN_W, BTN_H, COLOR_BTN); drawRect(BTN_SOLVE_X+17, BTN_SOLVE_Y+6, 2, 7, COLOR_BTN_TXT); drawRect(BTN_SOLVE_X+17, BTN_SOLVE_Y+14, 2, 2, COLOR_BTN_TXT);
}
void drawX(int x, int y, int s, u16 c) { for(int i=0; i<s; i++) { plot(x+i, y+i, c); plot(x+s-1-i, y+i, c); } }
void drawXMarker(int x, int y) { drawX(x+2, y+2, CELL_SIZE-5, COLOR_MARKER); }
void drawCursor(int r, int c, u16 color) {
    int x = MARGIN_LEFT + (c * CELL_SIZE); int y = MARGIN_TOP + (r * CELL_SIZE);
    drawRect(x, y, CELL_SIZE, 1, color); drawRect(x, y+CELL_SIZE-1, CELL_SIZE, 1, color); drawRect(x, y, 1, CELL_SIZE, color); drawRect(x+CELL_SIZE-1, y, 1, CELL_SIZE, color);
}
void renderGame(int cursorR, int cursorC) {
    ColorTheme* t = &themes[currentThemeIdx]; u16 bg = gameWon ? COLOR_WIN_BG : t->bg;
    for(int i=0; i<256*192; i++) bottomScreenBuf[i] = bg;
    for(int c=0; c<GRID_COLS; c++) {
        u16 txtColor = colDone[c] ? t->clue_done : t->clue_txt; int count = colClues[c].count; int x = MARGIN_LEFT + (c * CELL_SIZE) + 3; int yBase = MARGIN_TOP - 2;
        for(int i=count-1; i>=0; i--) drawMiniNum(x, yBase - ((count-1-i)*7) - 6, colClues[c].values[i], txtColor);
    }
    for(int r=0; r<GRID_ROWS; r++) {
        u16 txtColor = rowDone[r] ? t->clue_done : t->clue_txt; int count = rowClues[r].count; int y = MARGIN_TOP + (r * CELL_SIZE) + 3; int xBase = MARGIN_LEFT - 2;
        for(int i=count-1; i>=0; i--) { int val = rowClues[r].values[i]; int offset = (val > 9) ? 8 : 4; drawMiniNum(xBase - offset, y, val, txtColor); xBase -= (offset + 3); }
    }
    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            int px = MARGIN_LEFT + (c * CELL_SIZE); int py = MARGIN_TOP + (r * CELL_SIZE); u16 color = t->bg; if (playerGrid[r][c] == 1) color = t->filled;
            drawRect(px, py, CELL_SIZE-1, CELL_SIZE-1, color); drawRect(px+CELL_SIZE-1, py, 1, CELL_SIZE, t->grid); drawRect(px, py+CELL_SIZE-1, CELL_SIZE, 1, t->grid);
            if (playerGrid[r][c] == 2) drawXMarker(px, py);
        }
    }
    drawRect(MARGIN_LEFT-1, MARGIN_TOP-1, 1, (GRID_ROWS*CELL_SIZE)+1, t->clue_txt); drawRect(MARGIN_LEFT-1, MARGIN_TOP-1, (GRID_COLS*CELL_SIZE)+1, 1, t->clue_txt);
    if (!gameWon && cursorR >= 0) drawCursor(cursorR, cursorC, t->cursor); if (!gameWon) drawButtons();
}
void triggerExplosion() {
    fireworksActive = true;
    for(int i=0; i<MAX_PARTICLES; i++) {
        particles[i].active = true; particles[i].x = 128; particles[i].y = 80; particles[i].life = 60 + (rand() % 60);
        int r = rand() % 31; int g = rand() % 31; int b = rand() % 31; particles[i].color = RGB15(r, g, b) | BIT(15);
        particles[i].vx = ((rand() % 100) / 20.0) - 2.5; particles[i].vy = ((rand() % 100) / 20.0) - 3.5;
    }
}
void updateAndDrawFireworks() {
    int activeCount = 0;
    for(int i=0; i<MAX_PARTICLES; i++) {
        if(!particles[i].active) continue; activeCount++; particles[i].x += particles[i].vx; particles[i].y += particles[i].vy; particles[i].vy += 0.08; particles[i].life--;
        if(particles[i].life <= 0 || particles[i].y > 192 || particles[i].x < 0 || particles[i].x > 256) { particles[i].active = false; continue; }
        int px = (int)particles[i].x; int py = (int)particles[i].y; plot(px, py, particles[i].color); plot(px+1, py, particles[i].color); plot(px, py+1, particles[i].color); plot(px+1, py+1, particles[i].color);
    }
    if(activeCount == 0) fireworksActive = false;
}
void saveGame() { if (!fatReady) return; FILE* file = fopen("fat:/pikranji.sav", "wb"); if (file) { fwrite(&saveData, sizeof(SaveData), 1, file); fclose(file); } }
void loadGame() {
    saveData.score = 0; saveData.unlockedLimit = 10; saveData.solvedCount = 0; saveData.language = LANG_PT; for(int i=0; i<1000; i++) saveData.solved[i] = false;
    if (!fatReady) return; FILE* file = fopen("fat:/pikranji.sav", "rb");
    if (file) { fread(&saveData, sizeof(SaveData), 1, file); fclose(file); if (saveData.unlockedLimit < 10) saveData.unlockedLimit = 10; if (saveData.unlockedLimit > PUZZLE_COUNT) saveData.unlockedLimit = PUZZLE_COUNT; if (saveData.language != LANG_PT && saveData.language != LANG_EN) saveData.language = LANG_PT; } 
}
void refreshStaticText(int lang) {
    iprintf("\x1b[2J"); 
    if (lang == LANG_PT) { iprintf("\x1b[21;1H[?] Ajuda (Custo 1)"); iprintf("\x1b[21;20H[!] Resolver"); iprintf("\x1b[22;1H[X] Idioma: PT"); } 
    else { iprintf("\x1b[21;1H[?] Hint (Cost 1)"); iprintf("\x1b[21;20H[!] Solve"); iprintf("\x1b[22;1H[X] Lang: EN  "); }
}

// ============================================================================
// MAIN LOOP
// ============================================================================
int main(void) {
    irqEnable(IRQ_VBLANK);
    soundEnable(); 

    // --- CORREÇÃO DO LIXO VISUAL ---
    // Mapeamos a VRAM B para a Imagem (0x06020000)
    // Mapeamos a VRAM A para o Texto (0x06000000)
    videoSetMode(MODE_5_2D); 
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankB(VRAM_B_MAIN_BG_0x06020000);

    // Init Imagem (BG3) no VRAM B (Offset 128KB = MapBase 8)
    int bgTop = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 8, 0);
    topScreenBuf = bgGetGfxPtr(bgTop);
    
    // Init Texto (BG0) no VRAM A (Default) - Transparente por defeito
    consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

    // Sub Screen (Bottom)
    videoSetModeSub(MODE_5_2D); vramSetBankC(VRAM_C_SUB_BG);
    int bgBot = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bottomScreenBuf = bgGetGfxPtr(bgBot);

    // --- INIT ---
    initPuzzleSystem(); fatReady = fatInitDefault(); 
    soundPlaySample(music_bin, DATA_FORMAT, music_bin_size, SAMPLE_RATE, 80, 64, true, 0);
    dmaCopy(bgImages[0], topScreenBuf, 256*192*2);
    loadGame(); srand(time(NULL)); shuffleBag(); currentPuzzleIndex = getNextPuzzleID(); resetGame(); refreshStaticText(saveData.language);
    
    int lastR = -1, lastC = -1; bool forceRender = true;

    while(1) {
        swiIntrWait(1, IRQ_VBLANK);
        bgTimer++; if (bgTimer >= BG_SWITCH_TIME) { bgTimer = 0; switchBackground(); }
        updateSound(); scanKeys(); int held = keysHeld(); int down = keysDown();

        if (down & KEY_START) { resetGame(); forceRender = true; }
        if (down & KEY_SELECT) { currentPuzzleIndex = getNextPuzzleID(); resetGame(); forceRender = true; }
        if (down & KEY_X) { saveData.language = (saveData.language == LANG_PT) ? LANG_EN : LANG_PT; saveGame(); refreshStaticText(saveData.language); forceRender = true; }

        int currR = -1, currC = -1;
        if (down & KEY_TOUCH) {
            touchPosition touch; touchRead(&touch);
            if (!gameWon) {
                if (touch.px >= BTN_HINT_X && touch.px <= BTN_HINT_X + BTN_W && touch.py >= BTN_HINT_Y && touch.py <= BTN_HINT_Y + BTN_H) { useHint(); forceRender = true; }
                else if (touch.px >= BTN_SOLVE_X && touch.px <= BTN_SOLVE_X + BTN_W && touch.py >= BTN_SOLVE_Y && touch.py <= BTN_SOLVE_Y + BTN_H) { solvePuzzle(); forceRender = true; }
            }
            int c = (touch.px - MARGIN_LEFT) / CELL_SIZE; int r = (touch.py - MARGIN_TOP) / CELL_SIZE;
            if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS) {
                currR = r; currC = c;
                if (!gameWon) { isDragging = true; if (held & KEY_UP) dragType = (playerGrid[r][c] == 2) ? 2 : 3; else if (held & KEY_DOWN) dragType = (playerGrid[r][c] == 1) ? 2 : 1; else dragType = 0; }
            }
        } else if (held & KEY_TOUCH && isDragging) {
             touchPosition touch; touchRead(&touch); int c = (touch.px - MARGIN_LEFT) / CELL_SIZE; int r = (touch.py - MARGIN_TOP) / CELL_SIZE;
             if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS) {
                currR = r; currC = c;
                if (dragType != 0) {
                    int newState = -1; if (dragType == 1) newState = 1; else if (dragType == 2) newState = 0; else if (dragType == 3) newState = 2; 
                    if (playerGrid[r][c] != newState) { playerGrid[r][c] = newState; if (newState == 1) playSound(0); else if (newState == 2) playSound(1); else if (newState == 0) playSound(2); checkWin(); forceRender = true; }
                }
             }
        } else { isDragging = false; dragType = 0; }

        if (forceRender || currR != lastR || currC != lastC || fireworksActive) {
            const Puzzle* p = &allPuzzles[currentPuzzleIndex]; int L = saveData.language;
            iprintf("\x1b[2;2H-- PIKRANJI DS --"); 
            iprintf("\x1b[2;20H%s: %d     ", UI_TEXT[L][0], saveData.score); 
            iprintf("\x1b[14;2H%s: %d/%d   ", UI_TEXT[L][1], saveData.unlockedLimit, PUZZLE_COUNT);
            if (fatReady && saveData.solved[currentPuzzleIndex]) iprintf("\x1b[14;20H\x1b[33m[%s]\x1b[39m", UI_TEXT[L][2]);
            else if (cheated) iprintf("\x1b[14;20H\x1b[31m[%s]   \x1b[39m", UI_TEXT[L][3]);
            else iprintf("\x1b[14;20H           ");
            iprintf("\x1b[16;2H%s:                                ", UI_TEXT[L][4]);
            const char* meaningStr = (L == LANG_EN) ? p->meaning_en : p->meaning_pt; iprintf("\x1b[16;2H%s: \x1b[32m%s\x1b[39m", UI_TEXT[L][4], meaningStr);
            if (gameWon) iprintf("\x1b[18;2H** %s ** ", UI_TEXT[L][5]); else iprintf("\x1b[18;2H                  ");
            renderGame(currR, currC);
            if(fireworksActive) updateAndDrawFireworks();
            lastR = currR; lastC = currC; forceRender = false;
        }
    }
    return 0;
}
