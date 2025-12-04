.section .rodata
    .global music_bin
    .global music_bin_size
    .align 4

# Define o início dos dados
music_bin:
    # Importa o ficheiro binário diretamente (o caminho é relativo à pasta build/)
    .incbin "../assets/music.bin"

# Define o fim e calcula o tamanho
music_bin_end:
    .align 4

# Cria uma variável com o tamanho total
music_bin_size:
    .word music_bin_end - music_bin
