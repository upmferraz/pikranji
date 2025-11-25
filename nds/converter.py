import json

# Configurações
INPUT_FILE = 'puzzles.json'
OUTPUT_FILE = 'puzzles.h'
GRID_SIZE = 15

def generate_header():
    try:
        with open(INPUT_FILE, 'r', encoding='utf-8') as f:
            puzzles = json.load(f)
    except FileNotFoundError:
        print(f"Erro: Não encontrei o ficheiro {INPUT_FILE}")
        return

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
            f.write("    {\n")
            # Nota: O suporte a Kanji na DS exige fontes customizadas, 
            # mas guardamos a string aqui na mesma.
            f.write(f'        "{p["kanji"]}",\n') 
            f.write(f'        "{p["meaning"]}",\n')
            f.write("        {\n")
            
            # Escrever a grelha
            for row in p["grid"]:
                line_str = ", ".join(str(cell) for cell in row)
                f.write(f"            {{{line_str}}},\n")
            
            f.write("        }\n")
            f.write("    },\n" if idx < len(puzzles) - 1 else "    }\n")

        f.write("};\n\n")
        f.write("#endif // PUZZLES_H\n")

    print(f"Sucesso! {len(puzzles)} puzzles convertidos para {OUTPUT_FILE}.")

if __name__ == "__main__":
    generate_header()
