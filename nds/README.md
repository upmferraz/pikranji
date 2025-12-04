# Pikranji - Nintendo DS Port

Este é um port do jogo "Pikranji" (Picross com Kanji) para a Nintendo DS, escrito em C utilizando as bibliotecas **devkitPro** (`libnds` + `libcalico`).

## 📂 Estrutura do Projeto

* **`source/`**: Código fonte (`main.c` e lógica do jogo).
* **`include/`**: Ficheiros de cabeçalho (`puzzles.h`, gerado do JSON original).
* **`Makefile`**: Configuração de compilação baseada nas regras oficiais (`ds_rules`).
* **`build.sh`**: Script utilitário para compilar usando Docker (sem instalar nada no PC).

## 🛠️ Pré-requisitos

* **Docker** (Obrigatório para compilar sem dores de cabeça).
* **DeSmuME** ou **MelonDS** (Para testar/jogar).

## 🚀 Como Compilar

Não precisas de instalar o toolchain devkitPro no teu sistema. O script `build.sh` usa um contentor Docker oficial para fazer o trabalho sujo.

1.  **Dar permissão ao script (apenas na primeira vez):**
    ```bash
    chmod +x build.sh
    ```

2.  **Compilar o jogo:**
    ```bash
    ./build.sh
    ```
    *Isto irá gerar o ficheiro `pikranji.nds` na raiz.*

3.  **Limpar e Recompilar (Clean Build):**
    Se mudares nomes de ficheiros ou tiveres erros estranhos:
    ```bash
    ./build.sh clean
    ```

## 🎮 Como Jogar

Abre o ficheiro `.nds` gerado na tua consola ou emulador.

# Nota
Esta aplicação foi amplamente desenvolvida recorrendo a inteligência artificial, no caso foi utilizado o Gemini 2.5 e Gemini 3 Pro.
A música, fotografia são da minha autoria, para a música recorri ao software LMMS. Os melhores Kanji aqui gerados foram gerados manualmente recorrendo ao gerador incluído no software em HTML
