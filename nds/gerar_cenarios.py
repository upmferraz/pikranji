from PIL import Image, ImageEnhance
import os
import sys
import struct

# Configurações
INPUT_IMAGE = "mar.png"
OUTPUT_DIR = "assets"
TARGET_SIZE = (256, 192)

def rgb_to_ds(r, g, b):
    # Converte RGB888 (PC) para RGB555 (Nintendo DS)
    # Bits: 1 (Alpha) | 5 (Azul) | 5 (Verde) | 5 (Vermelho)
    R = (r >> 3) & 0x1F
    G = (g >> 3) & 0x1F
    B = (b >> 3) & 0x1F
    return 0x8000 | (B << 10) | (G << 5) | R

def save_as_bin(image, filename):
    # Converte a imagem para dados binários raw (Little Endian)
    pixels = image.load()
    with open(filename, 'wb') as f:
        for y in range(image.height):
            for x in range(image.width):
                r, g, b = pixels[x, y]
                val = rgb_to_ds(r, g, b)
                f.write(struct.pack('<H', val)) # Escreve 2 bytes (ushort)

def gerar_binarios(input_path):
    print(f"🌊 A processar: {input_path}")

    try:
        original = Image.open(input_path).convert('RGB')
    except FileNotFoundError:
        print(f"❌ Erro: '{input_path}' não encontrado.")
        return

    resized = original.resize(TARGET_SIZE, Image.Resampling.LANCZOS)
    
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    # --- VARIAÇÃO 1: DIA ---
    save_as_bin(resized, os.path.join(OUTPUT_DIR, "sea1.bin"))
    print("✅ data/sea1.bin gerado.")

    # --- VARIAÇÃO 2: ENTARDECER ---
    enhancer = ImageEnhance.Brightness(resized)
    dusk = enhancer.enhance(0.7)
    color_enhancer = ImageEnhance.Color(dusk)
    dusk = color_enhancer.enhance(1.2)
    save_as_bin(dusk, os.path.join(OUTPUT_DIR, "sea2.bin"))
    print("✅ data/sea2.bin gerado.")

    # --- VARIAÇÃO 3: NOITE ---
    night = enhancer.enhance(0.4)
    pixels = night.load()
    for y in range(night.height):
        for x in range(night.width):
            r, g, b = pixels[x, y]
            pixels[x, y] = (int(r * 0.6), int(g * 0.7), int(b * 1.1))
    
    save_as_bin(night, os.path.join(OUTPUT_DIR, "sea3.bin"))
    print("✅ data/sea3.bin gerado.")

    print("\n🎉 Ficheiros .bin prontos! O Makefile vai detetá-los agora.")

if __name__ == "__main__":
    file_to_convert = sys.argv[1] if len(sys.argv) > 1 else INPUT_IMAGE
    gerar_binarios(file_to_convert)
