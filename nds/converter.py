import json
import os
import unicodedata

# Configurações
# Agora aponta para o diretório anterior
INPUT_FILE = os.path.join('..', 'puzzles.json') 
OUTPUT_FILE = 'include/puzzles.h' # Garante que vai para a pasta include
GRID_SIZE = 15

def remove_accents(input_str):
    """
    Remove acentos e caracteres especiais para garantir compatibilidade
    com a fonte ASCII padrão da Nintendo DS.
    Ex: 'Três' -> 'Tres', 'Mãe' -> 'Mae'
    """
    if not isinstance(input_str, str):
        return str(input_str)
        
    # Normaliza para decompor caracteres (ex: 'ê' vira 'e' + '^')
    nfkd_form = unicodedata.normalize('NFKD', input_str)
    # Filtra apenas os caracteres que não são marcas de combinação (acentos)
    return "".join([c for c in nfkd_form if not unicodedata.combining(c)])

def generate_header():
    print(f"A procurar ficheiro em: {os.path.abspath(INPUT_FILE)}")
    
    try:
        with open(INPUT_FILE, 'r', encoding='utf-8') as f:
            puzzles = json.load(f)
    except FileNotFoundError:
        print(f"ERRO CRÍTICO: Não encontrei o '{INPUT_FILE}'")
        print("Certifica-te que o ficheiro puzzles.json está na pasta acima da pasta do projeto NDS.")
        return

    # Garante que a pasta include existe
    os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)

    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        # Cabeçalho do ficheiro C
        f.write("#ifndef PUZZLES_H\n")
        f.write("#define PUZZLES_H\n\n")
        f.write(f"#define PUZZLE_COUNT {len(puzzles)}\n\n")
        
        # Definição da Struct
        f.write("typedef struct {\n")
        f.write("    const char* kanji;\n")
        f.write("    const char* meaning;\n")
        f.write(f"    const int grid[{GRID_SIZE}][{GRID_SIZE}];\n")
        f.write("} Puzzle;\n\n")

        # Início do Array de Puzzles
        f.write("static const Puzzle allPuzzles[PUZZLE_COUNT] = {\n")

        for idx, p in enumerate(puzzles):
            # Limpeza dos dados
            # Nota: O Kanji provavelmente não vai aparecer bem na DS sem fonte customizada,
            # mas mantemos o original aqui. O meaning é limpo de acentos.
            kanji_clean = p["kanji"] 
            meaning_clean = remove_accents(p["meaning"])

            f.write("    {\n")
            f.write(f'        "{kanji_clean}",\n') 
            f.write(f'        "{meaning_clean}",\n') # Aqui usamos a versão sem acentos
            f.write("        {\n")
            
            # Escrever a grelha
            for row in p["grid"]:
                line_str = ", ".join(str(cell) for cell in row)
                f.write(f"            {{{line_str}}},\n")
            
            f.write("        }\n")
            f.write("    },\n" if idx < len(puzzles) - 1 else "    }\n")

        f.write("};\n\n")
        f.write("#endif // PUZZLES_H\n")

    print(f"✅ Sucesso! {len(puzzles)} puzzles convertidos e limpos para {OUTPUT_FILE}.")

if __name__ == "__main__":
    generate_header()
