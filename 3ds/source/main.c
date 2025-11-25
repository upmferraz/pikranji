#include <nds.h>
#include <stdio.h>
#include "puzzles.h" // O ficheiro que geraste com o Python

// ============================================================================
// CONFIGURAÇÕES VISUAIS
// ============================================================================
// A DS tem 256x192 pixeis.
// Vamos definir o tamanho das células para caberem 15x15 no ecrã.
#define CELL_SIZE 10 
#define GRID_START_X 50 // Margem esquerda para centrar
#define GRID_START_Y 20 // Margem superior

// Cores em formato 15-bit (RGB15)
// A macro é RGB15(vermelho, verde, azul) onde cada um vai de 0 a 31
#define COLOR_WHITE RGB15(31, 31, 31)
#define COLOR_BLACK RGB15(0, 0, 0)
#define COLOR_GRID  RGB15(20, 20, 20) // Cinzento claro
#define COLOR_BG    RGB15(28, 28, 28) // Fundo "papel"

// Ponteiro para a memória de vídeo do ecrã de baixo (Sub Screen)
u16* videoBuffer;

// ============================================================================
// FUNÇÕES DE DESENHO (O teu "Canvas")
// ============================================================================

// Desenha um píxel na memória de vídeo
void drawPixel(int x, int y, u16 color) {
    // Verificação de segurança para não desenhar fora do ecrã e crashar
    if (x >= 0 && x < 256 && y >= 0 && y < 192) {
        // A memória de vídeo é um array linear. 
        // A posição é: (linha atual * largura total) + coluna atual
        videoBuffer[y * 256 + x] = color;
    }
}

// Desenha um retângulo preenchido (uma célula da grelha)
void drawRect(int x, int y, int width, int height, u16 color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            drawPixel(x + j, y + i, color);
        }
    }
}

// Renderiza o puzzle completo
void drawPuzzle(const Puzzle* p) {
    // 1. Limpar o ecrã (pintar tudo com a cor de fundo)
    for (int i = 0; i < 256 * 192; i++) {
        videoBuffer[i] = COLOR_BG;
    }

    // 2. Desenhar a grelha
    for (int row = 0; row < 15; row++) {
        for (int col = 0; col < 15; col++) {
            
            // Calcular posição no ecrã (HTML: x, y)
            int screenX = GRID_START_X + (col * CELL_SIZE);
            int screenY = GRID_START_Y + (row * CELL_SIZE);

            // Ler do ficheiro .h (0 ou 1)
            int cellValue = p->grid[row][col];
            
            // Decidir cor
            u16 color = (cellValue == 1) ? COLOR_BLACK : COLOR_WHITE;

            // Desenhar a célula (com 1px a menos para criar o efeito de grelha)
            drawRect(screenX, screenY, CELL_SIZE - 1, CELL_SIZE - 1, color);
        }
    }
}

// ============================================================================
// LOOP PRINCIPAL
// ============================================================================
int main(void) {
    
    // 1. Inicializar Ecrã de Cima (Texto/Consola)
    // O consoleDemoInit configura automaticamente o ecrã de cima para texto simples
    consoleDemoInit();

    // 2. Inicializar Ecrã de Baixo (Gráficos)
    // MODE_5_2D permite usar bitmaps de 16-bit (cores reais)
    videoSetModeSub(MODE_5_2D);
    
    // Alocar o banco de memória VRAM C para o fundo do ecrã de baixo
    vramSetBankC(VRAM_C_SUB_BG);

    // Configurar o Background 3 no modo Bitmap 16-bit
    int bg3 = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    
    // Obter o endereço onde podemos escrever os pixeis
    videoBuffer = bgGetGfxPtr(bg3);

    // 3. Carregar o primeiro puzzle
    // Vamos buscar ao array 'allPuzzles' que está no puzzles.h
    const Puzzle* currentPuzzle = &allPuzzles[0];

    // Escrever info no ecrã de cima
    iprintf("\x1b[10;10HPIKRANJI DS"); // Move cursor para linha 10, col 10
    iprintf("\x1b[12;5HSignificado: %s", currentPuzzle->meaning);
    iprintf("\x1b[14;5H(Kanji nao suportado ainda)");

    // Desenhar a grelha no ecrã de baixo
    drawPuzzle(currentPuzzle);

    // 4. Loop Infinito (Game Loop)
    while(1) {
        // Esperar pela sincronização vertical (60 FPS)
        // Isto poupa bateria e evita "tearing" na imagem
	swiIntrWait(1, IRQ_VBLANK); 
        // Detetar input (para fechares se tiveres num emulador)
        scanKeys();
        int keys = keysDown();
        if (keys & KEY_START) break;
    }

    return 0;
}
