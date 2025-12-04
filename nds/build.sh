#!/bin/bash

# Nome da imagem Docker que criámos
IMAGE_NAME="nds-builder"

# Se passares o argumento "clean", ele limpa antes
if [ "$1" == "clean" ]; then
    echo "🧹 A limpar builds anteriores..."
    sudo rm -rf build/*
    sudo docker run --rm -v $(pwd):/source -w /source $IMAGE_NAME make clean
fi

echo "🚀 A compilar Pikranji..."
# O comando mágico que funcionou
sudo docker run --rm -v $(pwd):/source -w /source $IMAGE_NAME make

# Verifica se o ficheiro foi criado
if [ -f "source.nds" ]; then
    echo "✅ Sucesso! O ficheiro 'source.nds' está pronto."
    # Opcional: Renomear para algo mais bonito
    mv source.nds pikranji.nds
    echo "👉 Renomeado para 'pikranji.nds'"
else
    echo "❌ Erro na compilação."
fi
