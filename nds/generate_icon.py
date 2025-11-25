import struct

def create_ds_icon():
    width, height = 32, 32
    
    # Paleta de 16 cores (RGB)
    # 0: Transparente (Magenta), 1: Azul Fundo, 2: Branco, 3: Laranja, 4: Preto
    palette = [
        (255, 0, 255), # 0: Transparente
        (0, 128, 255), # 1: Azul "Nintendo"
        (255, 255, 255), # 2: Branco
        (255, 100, 0),   # 3: Laranja
        (20, 20, 20),    # 4: Preto
    ] + [(0,0,0)] * 11 # Encher o resto com preto

    # Desenhar o Ícone (Matriz 32x32)
    # 1=Fundo, 2=Grelha/Kanji, 3=Destaque
    pixels = [[1 for _ in range(width)] for _ in range(height)]

    # Desenhar borda e grelha (Kanji de "Campo" / Grelha de Picross)
    for y in range(4, 28):
        for x in range(4, 28):
            # Borda grossa
            if x < 6 or x > 25 or y < 6 or y > 25:
                pixels[y][x] = 4 
            # Cruz central
            elif (14 <= x <= 17) or (14 <= y <= 17):
                pixels[y][x] = 4
            # Preenchimento
            else:
                pixels[y][x] = 2 # Branco

    # Adicionar um quadrado "pintado" (Laranja) no canto superior esquerdo
    for y in range(7, 13):
        for x in range(7, 13):
            pixels[y][x] = 3

    # --- Escrever BMP (Formato RLE ou não comprimido) ---
    # Header BMP básico para 4bpp (16 cores)
    filesize = 14 + 40 + (16 * 4) + (32 * 32 // 2)
    offset = 14 + 40 + (16 * 4)
    
    with open("icon.bmp", "wb") as f:
        # File Header
        f.write(b'BM')
        f.write(struct.pack('<I', filesize))
        f.write(b'\x00\x00\x00\x00')
        f.write(struct.pack('<I', offset))
        
        # Info Header
        f.write(struct.pack('<I', 40)) # Header Size
        f.write(struct.pack('<i', width))
        f.write(struct.pack('<i', height * -1)) # Top-down
        f.write(struct.pack('<H', 1)) # Planes
        f.write(struct.pack('<H', 4)) # Bit count (4bpp)
        f.write(struct.pack('<I', 0)) # Compression (BI_RGB)
        f.write(struct.pack('<I', 0)) # Image Size
        f.write(struct.pack('<i', 0)) # X pixels per meter
        f.write(struct.pack('<i', 0)) # Y pixels per meter
        f.write(struct.pack('<I', 16)) # Colors used
        f.write(struct.pack('<I', 0)) # Important colors

        # Color Table (BGR + Reserved)
        for r, g, b in palette:
            f.write(struct.pack('BBBB', b, g, r, 0))

        # Pixel Data (Packed 2 pixels per byte)
        for y in range(height):
            row = pixels[y]
            for x in range(0, width, 2):
                px1 = row[x] & 0x0F
                px2 = row[x+1] & 0x0F
                byte = (px1 << 4) | px2
                f.write(struct.pack('B', byte))

    print("✅ Ícone 'icon.bmp' criado com sucesso!")

if __name__ == "__main__":
    create_ds_icon()
