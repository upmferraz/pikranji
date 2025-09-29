## 🛠️ Gerador de JSON para Pikranji (Kanji Picross)

-----

## 🎯 Objetivo

Este gerador é uma ferramenta simples baseada em navegador, projetada para facilitar a criação de novos puzzles de Kanji (Nonogram/Picross 15x15) e formatá-los no padrão JSON exigido pelo jogo principal **Pikranji**.

Com esta ferramenta, pode desenhar visualmente o Kanji e exportar um ficheiro `kanji_collection.json` pronto a ser adicionado à lista de puzzles.

-----

## 🚀 Como Usar o Gerador

Siga estes passos simples para criar e exportar novos puzzles:

### 1\. Configurar o Kanji

No topo da página, preencha a informação do puzzle que está a criar:

  * **Kanji (Caractere):** Insira o caractere japonês que está a desenhar (ex: `木`).
  * **Significado:** Insira o significado em Português para este caractere (ex: `Árvore`).

### 2\. Desenhar na Grelha

Utilize a grelha de desenho 15x15 para criar a sua imagem.

  * **Clique/Toque:** Clique ou toque numa célula para alternar o seu estado entre **vazio (`0`)** e **preenchido (`1`)**.
  * **Arrastar (Drag):** Mantenha o botão do mouse ou o dedo pressionado e arraste para preencher rapidamente várias células.

### 3\. Adicionar à Coleção

Após terminar o desenho:

1.  Clique no botão **Adicionar à Coleção**.
2.  O puzzle será guardado na lista de puzzles, e a grelha de desenho será automaticamente limpa, pronta para um novo desenho.
3.  O seu novo puzzle aparecerá na secção "Puzzles na Coleção" no fundo da página.

### 4\. Exportar o Ficheiro JSON

Quando tiver adicionado todos os Kanijis que deseja criar:

1.  Clique no botão **Descarregar JSON (`kanji_collection.json`)**.
2.  Será gerado um ficheiro JSON formatado e pronto a ser utilizado.

-----

## 📋 Estrutura de Exportação

O ficheiro gerado segue rigorosamente o formato necessário pelo jogo **Pikranji**:

```json
[
    {
        "kanji": "木",
        "meaning": "Árvore",
        "grid": [
            [0,0,0,0,0,0,1,0,0,0,0,0,0,0,0],
            // ... 15 linhas no total ...
            [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
        ]
    }
]
```

### Notas Importantes:

  * **Células Ativas:** As células preenchidas no desenho são representadas pelo valor **`1`** no JSON.
  * **Células Vazias:** As células vazias são representadas pelo valor **`0`** no JSON.
  * Pode copiar o conteúdo do ficheiro `kanji_collection.json` e colá-lo diretamente no seu ficheiro `puzzles.json` do jogo principal.
