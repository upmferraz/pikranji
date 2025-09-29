# Pikranji - O Kanji Picross

-----

## 🎯 Objetivo do Jogo

**Pikranji** é uma versão do popular jogo de lógica Picross (também conhecido como Nonogram ou Griddlers), onde o objetivo é revelar uma imagem oculta numa grelha.

Neste jogo, a imagem oculta é sempre um **Kanji** (caractere japonês), e o seu significado é apresentado logo abaixo da grelha para dar uma pista.

Para vencer, deve preencher corretamente as células da grelha de 15x15, seguindo as pistas numéricas fornecidas nas laterais e no topo.

Este jogo tem principalmente um objetivo didático, que é o de uma forma gamificada, aprender os milhares de caracteres Kanji existentes. Sinta-se livre em adicionar os que não estão incluídos com recurso ao gerador.

-----

## 🔍 Como Jogar

O tabuleiro é composto por uma grelha central e duas áreas de pistas:

  * **Pistas Laterais (Esquerda):** Indicam o número e o tamanho dos blocos de células preenchidas (pretas) em cada **linha**.
  * **Pistas Superiores (Topo):** Indicam o número e o tamanho dos blocos de células preenchidas (pretas) em cada **coluna**.

**Exemplo:**
Se uma pista de linha for `[3 5 1]`, isso significa que essa linha contém:

1.  Um bloco de 3 células preenchidas.
2.  Pelo menos uma célula vazia/marcada.
3.  Um bloco de 5 células preenchidas.
4.  Pelo menos uma célula vazia/marcada.
5.  Um bloco de 1 célula preenchida.

### 🛠️ Modos de Interação (Ferramentas)

Pode alternar entre duas ferramentas usando os botões **`⬛` (Preencher)** e **`❌` (Marcar)**:

| Ferramenta | Ícone | Uso Principal |
| :--- | :--- | :--- |
| **Preencher** | `⬛` | Marca uma célula como preenchida (cor preta). |
| **Marcar** | `❌` | Marca uma célula com um **`×`** (para indicar que deve ficar vazia). |

### 🖱️ Interação por Rato

  * **Clique Rápido:** Alterna o estado da célula (toggle) de acordo com a ferramenta selecionada.
  * **Arrastar (Drag):** Mantém o botão do rato premido e arrasta. A primeira célula clicada define a ação para todas as células seguintes.

### 📱 Interação por Toque (Mobile)

  * **Toque Rápido (Tap):** Ativa o modo de alternância (toggle) na célula, de acordo com a ferramenta selecionada.
  * **Arrastar (Drag/Swipe):** O movimento imediato após o toque ativa o modo de arrasto para preencher/marcar várias células.
  * **Toque Demorado (Long Press - \~0.5s):** Funciona como um toque rápido (toggle), útil para garantir que um toque mais lento seja registado.

### Botões de Controlo

  * **Reiniciar:** Limpa a grelha atual e volta ao estado inicial, mantendo o mesmo Kanji.
  * **Resolver:** Revela a solução correta (pode ser usado como último recurso\!).
  * **Próximo Kanji:** Carrega um novo puzzle e um novo Kanji aleatório.

-----

## ✅ Pistas Concluídas

Quando as células preenchidas numa linha ou coluna corresponderem exatamente aos números da pista dessa linha/coluna, as pistas numéricas serão riscadas, indicando que a linha/coluna está **concluída**.

**O jogo termina e você vence** quando todas as linhas e colunas estiverem concluídas e a imagem do Kanji estiver totalmente revelada.

-----

## 🚀 Adicionar Novos Puzzles (Kanji)

Pode facilmente adicionar novos desafios ao jogo editando diretamente o ficheiro **`puzzles.json`**. Este ficheiro é um *Array* de objetos JSON, onde cada objeto representa um puzzle completo.

### Estrutura de um Puzzle

Cada objeto de puzzle deve seguir o formato abaixo e ser anexado ao array principal:

| Campo | Tipo | Descrição |
| :--- | :--- | :--- |
| `"kanji"` | String | O caractere japonês que será exibido. |
| `"meaning"` | String | O significado do Kanji em Português. |
| `"grid"` | Array (15x15) | A grelha de solução. Deve ser uma matriz de 15 por 15, onde **`1`** representa uma célula preenchida (preto) e **`0`** representa uma célula vazia. |

### Exemplo de Código JSON

```json
{
                "kanji": "田",
                "meaning": "Campo de Arroz",
                "grid": [
                        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                        [0,0,1,1,1,1,1,1,1,1,1,1,1,0,0],
                        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
                        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
                        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
                        [0,0,1,1,1,1,1,1,1,1,1,1,1,0,0],
                        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
                        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
                        [0,0,1,0,0,0,0,1,0,0,0,0,1,0,0],
                        [0,0,1,1,1,1,1,1,1,1,1,1,1,0,0],
                        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                        [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                ]
}
```

Basta garantir que a `grid` tem exatamente **15 linhas** e cada linha tem **15 valores**. O jogo irá calcular automaticamente as pistas a partir desta grelha de solução.
