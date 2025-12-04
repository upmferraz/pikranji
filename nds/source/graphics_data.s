.section .rodata
    .align 4

# --- Imagem 1: Dia ---
    .global sea1_bin
sea1_bin:
    .incbin "../assets/sea1.bin"
    .align 4

# --- Imagem 2: Tarde ---
    .global sea2_bin
sea2_bin:
    .incbin "../assets/sea2.bin"
    .align 4

# --- Imagem 3: Noite ---
    .global sea3_bin
sea3_bin:
    .incbin "../assets/sea3.bin"
    .align 4
