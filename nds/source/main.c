#include <nds.h>
#include <stdio.h>
#include "puzzles.h"

// ============================================================================
// 🎨 ESTILO VISUAL "NINTENDO LITE"
// ============================================================================
// Cores com bit 15 (Opacidade) ligado
#define COLOR_BG        (RGB15(31, 31, 31) | BIT(15))  // Branco Puro
#define COLOR_GRID      (RGB15(24, 24, 24) | BIT(15))  // Cinza Muito Claro
#define COLOR_CLUE_TXT  (RGB15(10, 10, 10) | BIT(15))  // Cinza Escuro (Texto)
#define COLOR_FILLED    (RGB15(5,  5,  5)  | BIT(15))  // Preto Suave (Preenchido)
#define COLOR_MARKER    (RGB15(20, 0,  0)  | BIT(15))  // Vermelho Escuro (X)
#define COLOR_CURSOR    (RGB15(0,  20, 31) | BIT(15))  // Azul Ciano (Cursor)
#define COLOR_WIN_BG    (RGB15(25, 31, 25) | BIT(15))  // Verde Menta (Vitória)

// Configuração da Grelha
#define CELL_SIZE 10
#define GRID_COLS 15
#define GRID_ROWS 15
// Espaço reservado para as pistas (Margem esquerda e topo)
#define MARGIN_LEFT 60 
#define MARGIN_TOP  40

// Arrays globais
u16* videoBuffer;
int currentPuzzleIndex = 0;
int playerGrid[GRID_ROWS][GRID_COLS];
bool gameWon = false;

// Estruturas para as Dicas
#define MAX_CLUES 8
typedef struct { int count; int values[MAX_CLUES]; } LineClues;
LineClues rowClues[GRID_ROWS];
LineClues colClues[GRID_COLS];

// ============================================================================
// 🔊 ÁUDIO (PSG - Gameboy Style)
// ============================================================================
void playSound(int type) {
    // Canal, Duty Cycle (Timbre), Frequência (Agudo/Grave), Volume, Pan
    if (type == 0) { // Click Pintar (Grave)
        soundPlayPSG(DutyCycle_50, 400, 100, 64);
    } else if (type == 1) { // Click Marcar (Agudo e curto)
        soundPlayPSG(DutyCycle_25, 800, 80, 64);
    } else if (type == 2) { // Vitória (Arpeggio simples manual)
        soundPlayPSG(DutyCycle_50, 1000, 127, 64);
    } else if (type == 3) { // Apagar
        soundPlayPSG(DutyCycle_12, 200, 80, 64);
    }
}

// ============================================================================
// 🖌️ MOTOR GRÁFICO & FONTE MINI
// ============================================================================

void plot(int x, int y, u16 color) {
    if (x >= 0 && x < 256 && y >= 0 && y < 192) videoBuffer[y * 256 + x] = color;
}

void drawRect(int x, int y, int w, int h, u16 color) {
    for(int i=0; i<h; i++) for(int j=0; j<w; j++) plot(x+j, y+i, color);
}

// Mini-fonte 3x5 para desenhar números pequenos nas pistas
// Cada byte representa uma coluna de pixeis (LSB em cima)
const u8 miniFont[10][3] = {
    {0x1F, 0x11, 0x1F}, // 0
    {0x00, 0x1F, 0x00}, // 1 (Simplificado)
    {0x1D, 0x15, 0x17}, // 2
    {0x15, 0x15, 0x1F}, // 3
    {0x07, 0x04, 0x1F}, // 4
    {0x17, 0x15, 0x1D}, // 5
    {0x1F, 0x15, 0x1D}, // 6
    {0x01, 0x01, 0x1F}, // 7
    {0x1F, 0x15, 0x1F}, // 8
    {0x17, 0x15, 0x1F}  // 9
};

void drawMiniNum(int x, int y, int num, u16 color) {
    if (num > 9) { // Desenha dois digitos se > 9
        drawMiniNum(x - 4, y, num / 10, color);
        num %= 10;
    }
    for (int col = 0; col < 3; col++) {
        u8 colData = miniFont[num][col];
        for (int row = 0; row < 5; row++) {
            if ((colData >> row) & 1) plot(x + col, y + row, color);
        }
    }
}

// Desenha um X (Marcador)
void drawX(int x, int y, int s, u16 c) {
    for(int i=0; i<s; i++) { plot(x+i, y+i, c); plot(x+s-1-i, y+i, c); }
}

// Agora o drawXMarker já sabe o que é o drawX
void drawXMarker(int x, int y) {
    drawX(x + 2, y + 2, CELL_SIZE - 5, COLOR_MARKER);
}

// Desenha cursor (border highlight)
void drawCursor(int r, int c) {
    int x = MARGIN_LEFT + (c * CELL_SIZE);
    int y = MARGIN_TOP + (r * CELL_SIZE);
    drawRect(x, y, CELL_SIZE, 1, COLOR_CURSOR);
    drawRect(x, y+CELL_SIZE-1, CELL_SIZE, 1, COLOR_CURSOR);
    drawRect(x, y, 1, CELL_SIZE, COLOR_CURSOR);
    drawRect(x+CELL_SIZE-1, y, 1, CELL_SIZE, COLOR_CURSOR);
}

// ============================================================================
// 🧠 LÓGICA DO JOGO
// ============================================================================

// Gera as pistas baseado na grelha do puzzle atual
void calculateClues() {
    const Puzzle* p = &allPuzzles[currentPuzzleIndex];
    
    // Linhas
    for(int r=0; r<GRID_ROWS; r++) {
        int idx = 0, count = 0;
        for(int c=0; c<GRID_COLS; c++) {
            if(p->grid[r][c] == 1) count++;
            else if(count > 0) { if(idx<MAX_CLUES) rowClues[r].values[idx++] = count; count = 0; }
        }
        if(count > 0 && idx<MAX_CLUES) rowClues[r].values[idx++] = count;
        rowClues[r].count = (idx == 0) ? 0 : idx; // Se vazio, 0 pistas
    }

    // Colunas
    for(int c=0; c<GRID_COLS; c++) {
        int idx = 0, count = 0;
        for(int r=0; r<GRID_ROWS; r++) {
            if(p->grid[r][c] == 1) count++;
            else if(count > 0) { if(idx<MAX_CLUES) colClues[c].values[idx++] = count; count = 0; }
        }
        if(count > 0 && idx<MAX_CLUES) colClues[c].values[idx++] = count;
        colClues[c].count = (idx == 0) ? 0 : idx;
    }
}

void resetGame() {
    for(int i=0; i<GRID_ROWS; i++) 
        for(int j=0; j<GRID_COLS; j++) playerGrid[i][j] = 0;
    gameWon = false;
    calculateClues();
}

void checkWin() {
    if (gameWon) return;
    const Puzzle* p = &allPuzzles[currentPuzzleIndex];
    bool match = true;
    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            if (p->grid[r][c] == 1 && playerGrid[r][c] != 1) match = false;
            if (p->grid[r][c] == 0 && playerGrid[r][c] == 1) match = false;
        }
    }
    if (match) {
        gameWon = true;
        playSound(2); // Som Vitória
    }
}

void renderGame(int cursorR, int cursorC) {
    u16 bg = gameWon ? COLOR_WIN_BG : COLOR_BG;
    for(int i=0; i<256*192; i++) videoBuffer[i] = bg;

    // 1. Desenhar Pistas (Topo)
    for(int c=0; c<GRID_COLS; c++) {
        int count = colClues[c].count;
        int x = MARGIN_LEFT + (c * CELL_SIZE) + 3;
        int yBase = MARGIN_TOP - 2;
        for(int i=count-1; i>=0; i--) { // Desenha de baixo para cima
            drawMiniNum(x, yBase - ((count-1-i)*7) - 6, colClues[c].values[i], COLOR_CLUE_TXT);
        }
    }

    // 2. Desenhar Pistas (Esquerda)
    for(int r=0; r<GRID_ROWS; r++) {
        int count = rowClues[r].count;
        int y = MARGIN_TOP + (r * CELL_SIZE) + 3;
        int xBase = MARGIN_LEFT - 2;
        for(int i=count-1; i>=0; i--) { // Desenha da direita para a esquerda
            int val = rowClues[r].values[i];
            int offset = (val > 9) ? 8 : 4; // Espaço extra para 2 digitos
            drawMiniNum(xBase - offset, y, val, COLOR_CLUE_TXT);
            xBase -= (offset + 3);
        }
    }

    // 3. Desenhar Grelha
    for(int r=0; r<GRID_ROWS; r++) {
        for(int c=0; c<GRID_COLS; c++) {
            int px = MARGIN_LEFT + (c * CELL_SIZE);
            int py = MARGIN_TOP + (r * CELL_SIZE);
            
            // Fundo da célula (com margem para simular linha de grelha)
            u16 color = COLOR_BG; 
            if (playerGrid[r][c] == 1) color = COLOR_FILLED;
            
            drawRect(px, py, CELL_SIZE-1, CELL_SIZE-1, color);
            
            // Linhas da grelha (simuladas pelo fundo)
            drawRect(px+CELL_SIZE-1, py, 1, CELL_SIZE, COLOR_GRID);
            drawRect(px, py+CELL_SIZE-1, CELL_SIZE, 1, COLOR_GRID);

            if (playerGrid[r][c] == 2) drawXMarker(px, py);
        }
    }
    
    // Bordas principais da grelha
    drawRect(MARGIN_LEFT-1, MARGIN_TOP-1, 1, (GRID_ROWS*CELL_SIZE)+1, COLOR_CLUE_TXT); // Esquerda
    drawRect(MARGIN_LEFT-1, MARGIN_TOP-1, (GRID_COLS*CELL_SIZE)+1, 1, COLOR_CLUE_TXT); // Topo

    if (!gameWon && cursorR >= 0) drawCursor(cursorR, cursorC);
}

// ============================================================================
// MAIN LOOP
// ============================================================================
int main(void) {
    irqEnable(IRQ_VBLANK);
    
    // Inicializar Som (Obrigatório para o playSound funcionar)
    soundEnable(); 

    // Setup Ecrãs
    videoSetMode(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    consoleInit(0, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    int bg3 = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    videoBuffer = bgGetGfxPtr(bg3);

    resetGame();
    int lastR = -1, lastC = -1;
    bool forceRender = true;

    iprintf("\x1b[2;8H-- PIKRANJI DS --");
    iprintf("\x1b[4;2HControlos:");
    iprintf("\x1b[6;2H[Seta Baixo] Pintar");
    iprintf("\x1b[7;2H[Botao B]    Marcar (X)");
    iprintf("\x1b[8;2H[Stylus]     Mover");
    iprintf("\x1b[10;2H[Select]     Prox. Nivel");

    while(1) {
        swiIntrWait(1, IRQ_VBLANK);
        scanKeys();
        int held = keysHeld();
        int down = keysDown();

        if (down & KEY_START) { resetGame(); forceRender = true; }
        if (down & KEY_SELECT) {
            currentPuzzleIndex = (currentPuzzleIndex + 1) % PUZZLE_COUNT;
            resetGame();
            forceRender = true;
        }

        // Input do Jogo
        int currR = -1, currC = -1;
        if (held & KEY_TOUCH) {
            touchPosition touch;
            touchRead(&touch);
            int c = (touch.px - MARGIN_LEFT) / CELL_SIZE;
            int r = (touch.py - MARGIN_TOP) / CELL_SIZE;

            if (r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS) {
                currR = r; currC = c;
                
                if (!gameWon) {
                    if (held & KEY_DOWN) { // Pintar
                        if (playerGrid[r][c] != 1) {
                            playerGrid[r][c] = 1;
                            playSound(0); // Som de tinta
                            checkWin();
                            forceRender = true;
                        }
                    } else if (held & KEY_B) { // Marcar
                        if (playerGrid[r][c] != 2) {
                            playerGrid[r][c] = 2;
                            playSound(1); // Som de marcação
                            forceRender = true;
                        }
                    } else if (down & KEY_TOUCH) {
                        // Som subtil só para feedback de toque se não estiver a pintar
                        // playSound(3); // Opcional
                    }
                }
            }
        }

        if (forceRender || currR != lastR || currC != lastC) {
            const Puzzle* p = &allPuzzles[currentPuzzleIndex];
            iprintf("\x1b[14;2HPuzzle: %d / %d   ", currentPuzzleIndex + 1, PUZZLE_COUNT);
            
            // Só mostra o significado se ganhar (como spoiler protection!)
            if (gameWon) {
                iprintf("\x1b[16;2HSignificado:");
                iprintf("\x1b[17;2H\x1b[32m%s\x1b[39m", p->meaning); // Texto verde
                iprintf("\x1b[19;2H** COMPLETO! ** ");
            } else {
                iprintf("\x1b[16;2H               ");
                iprintf("\x1b[17;2H????????       ");
                iprintf("\x1b[19;2H                ");
            }

            renderGame(currR, currC);
            lastR = currR; lastC = currC;
            forceRender = false;
        }
    }
    return 0;
}
