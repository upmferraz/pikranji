import json
import os
import unicodedata

# Configurações
INPUT_FILE = os.path.join('..', 'puzzles.json') 
OUTPUT_FILE = 'include/puzzles.h' 
GRID_SIZE = 15

def remove_accents(input_str):
    """
    Remove acentos para compatibilidade com a fonte da NDS.
    """
    if not isinstance(input_str, str):
        return str(input_str)
    nfkd_form = unicodedata.normalize('NFKD', input_str)
    return "".join([c for c in nfkd_form if not unicodedata.combining(c)])

def generate_header():
    print(f"A procurar ficheiro em: {os.path.abspath(INPUT_FILE)}")
    
    try:
        with open(INPUT_FILE, 'r', encoding='utf-8') as f:
            puzzles = json.load(f)
    except FileNotFoundError:
        print(f"ERRO CRÍTICO: Não encontrei o '{INPUT_FILE}'")
        return

    os.makedirs(os.path.dirname(OUTPUT_FILE), exist_ok=True)

    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        f.write("#ifndef PUZZLES_H\n")
        f.write("#define PUZZLES_H\n\n")
        f.write(f"#define PUZZLE_COUNT {len(puzzles)}\n\n")
        
        # --- ALTERAÇÃO AQUI: Adicionado meaning_en ---
        f.write("typedef struct {\n")
        f.write("    const char* kanji;\n")
        f.write("    const char* meaning_pt;\n") # Renomeado para clareza
        f.write("    const char* meaning_en;\n") # Novo campo
        f.write(f"    const int grid[{GRID_SIZE}][{GRID_SIZE}];\n")
        f.write("} Puzzle;\n\n")

        f.write("static const Puzzle allPuzzles[PUZZLE_COUNT] = {\n")

        for idx, p in enumerate(puzzles):
            kanji_clean = p.get("kanji", "?")
            # Ler PT e EN, removendo acentos
            meaning_pt = remove_accents(p.get("meaning", "Sem dados"))
            # Fallback para PT se não houver EN
            meaning_en = remove_accents(p.get("meaning_en", meaning_pt)) 

            f.write("    {\n")
            f.write(f'        "{kanji_clean}",\n') 
            f.write(f'        "{meaning_pt}",\n') 
            f.write(f'        "{meaning_en}",\n') 
            f.write("        {\n")
            
            for row in p["grid"]:
                line_str = ", ".join(str(cell) for cell in row)
                f.write(f"            {{{line_str}}},\n")
            
            f.write("        }\n")
            f.write("    },\n" if idx < len(puzzles) - 1 else "    }\n")

        f.write("};\n\n")
        f.write("#endif // PUZZLES_H\n")

    print(f"✅ Sucesso! {len(puzzles)} puzzles convertidos (PT/EN) para {OUTPUT_FILE}.")

if __name__ == "__main__":
    generate_header()
