import json
import re
import os
from PIL import Image, ImageDraw, ImageFont

# --- CONFIGURAÇÃO ---
FONT_PATH = "NotoSansJP-Bold.ttf"
GRID_SIZE = 15
INPUT_FILE = "puzzles.json"      # O teu ficheiro atual
OUTPUT_FILE = "novos_puzzles.json" # O ficheiro novo a gerar

# --- 1. CARREGAR EXCLUSÕES ---
excluded_kanji = set()
if os.path.exists(INPUT_FILE):
    try:
        with open(INPUT_FILE, 'r', encoding='utf-8') as f:
            existing_data = json.load(f)
            for item in existing_data:
                excluded_kanji.add(item['kanji'])
        print(f"Ignorando {len(excluded_kanji)} Kanji já existentes.")
    except Exception as e:
        print(f"Aviso: Não consegui ler o {INPUT_FILE}. Vou gerar tudo. Erro: {e}")

# --- 2. LISTA MESTRA (N5 + N4) ---
# Removi os duplicados da lista anterior e expandi
kanji_master_list = [
    # N5 (Básico)
    ("一", "Um"), ("二", "Dois"), ("三", "Três"), ("四", "Quatro"), ("五", "Cinco"),
    ("六", "Seis"), ("七", "Sete"), ("八", "Oito"), ("九", "Nove"), ("十", "Dez"),
    ("百", "Cem"), ("千", "Mil"), ("万", "Dez Mil"), ("円", "Yen"), ("日", "Dia/Sol"),
    ("月", "Lua/Mês"), ("火", "Fogo"), ("水", "Água"), ("木", "Árvore"), ("金", "Ouro/Dinheiro"),
    ("土", "Terra"), ("曜", "Dia da semana"), ("年", "Ano"), ("上", "Cima"), ("下", "Baixo"),
    ("中", "Meio"), ("外", "Fora"), ("前", "Frente"), ("後", "Trás"), ("右", "Direita"),
    ("左", "Esquerda"), ("大", "Grande"), ("小", "Pequeno"), ("長", "Longo"), ("短", "Curto"),
    ("高", "Alto"), ("安", "Barato/Seguro"), ("新", "Novo"), ("古", "Velho"), ("多", "Muitos"),
    ("少", "Poucos"), ("行", "Ir"), ("来", "Vir"), ("帰", "Voltar"), ("食", "Comer"),
    ("飲", "Beber"), ("見", "Ver"), ("聞", "Ouvir"), ("読", "Ler"), ("書", "Escrever"),
    ("話", "Falar"), ("買", "Comprar"), ("教", "Ensinar"), ("会", "Encontrar"), ("休", "Descansar"),
    ("立", "Levantar"), ("座", "Sentar"), ("入", "Entrar"), ("出", "Sair"), ("待", "Esperar"),
    ("住", "Morar"), ("自", "Eu mesmo"), ("父", "Pai"), ("母", "Mãe"), ("子", "Criança"),
    ("男", "Homem"), ("女", "Mulher"), ("友", "Amigo"), ("人", "Pessoa"), ("名", "Nome"),
    ("目", "Olho"), ("耳", "Orelha"), ("口", "Boca"), ("手", "Mão"), ("足", "Pé"),
    ("天", "Céu"), ("気", "Espírito"), ("雨", "Chuva"), ("雪", "Neve"), ("電", "Eletricidade"),
    ("車", "Carro"), ("駅", "Estação"), ("道", "Estrada"), ("国", "País"), ("社", "Empresa"),
    ("校", "Escola"), ("本", "Livro"), ("川", "Rio"), ("山", "Montanha"), ("花", "Flor"),
    ("魚", "Peixe"), ("空", "Céu"), ("白", "Branco"), ("黒", "Preto"), ("赤", "Vermelho"),
    ("青", "Azul"), ("午", "Meio-dia"), ("今", "Agora"), ("週", "Semana"), ("時", "Hora"),
    ("間", "Intervalo"), ("分", "Minuto"), ("半", "Metade"), ("毎", "Todo"), ("何", "O quê"),
    
    # N4 (Adicionais)
    ("家", "Casa"), ("矢", "Flecha"), ("族", "Família"), ("親", "Pais"), ("兄", "Irmão +Velho"),
    ("姉", "Irmã +Velha"), ("弟", "Irmão +Novo"), ("妹", "Irmã +Nova"), ("私", "Eu"), ("夫", "Marido"),
    ("妻", "Esposa"), ("主", "Principal"), ("糸", "Fio"), ("紙", "Papel"), ("歌", "Canção"),
    ("写", "Copiar"), ("真", "Verdade"), ("工", "Construção"), ("広", "Largo"), ("店", "Loja"),
    ("病", "Doença"), ("院", "Instituição"), ("医", "Médico"), ("者", "Pessoa (Prof)"), ("死", "Morte"),
    ("去", "Passado"), ("味", "Sabor"), ("注", "Nota"), ("夏", "Verão"), ("秋", "Outono"),
    ("冬", "Inverno"), ("春", "Primavera"), ("京", "Capital"), ("都", "Metrópole"), ("堂", "Salão"),
    ("建", "Construir"), ("物", "Coisa"), ("館", "Edifício"), ("室", "Quarto"), ("屋", "Telhado"),
    ("図", "Mapa"), ("用", "Uso"), ("地", "Chão"), ("理", "Lógica"), ("科", "Departamento"),
    ("作", "Fazer"), ("泳", "Nadar"), ("海", "Mar"), ("野", "Campo"), ("通", "Passar"),
    ("運", "Sorte/Transporte"), ("転", "Rodar"), ("選", "Escolher"), ("洗", "Lavar"), ("止", "Parar"),
    ("歩", "Caminhar"), ("走", "Correr"), ("起", "Acordar"), ("寝", "Dormir"), ("貸", "Emprestar"),
    ("借", "Pedir Emprestado"), ("使", "Usar"), ("働", "Trabalhar"), ("売", "Vender"), ("知", "Saber"),
    ("思", "Pensar"), ("言", "Dizer"), ("計", "Medir"), ("試", "Tentar"), ("合", "Unir"),
    ("始", "Começar"), ("終", "Terminar"), ("開", "Abrir"), ("閉", "Fechar"), ("送", "Enviar"),
    ("切", "Cortar"), ("急", "Apressar"), ("乗", "Subir"), ("降", "Descer"), ("着", "Chegar/Vestir"),
    ("究", "Pesquisa"), ("研", "Afiar"), ("問", "Pergunta"), ("題", "Tópico"), ("習", "Aprender"),
    ("漢", "China"), ("字", "Caractere"), ("文", "Frase"), ("英", "Inglaterra"), ("質", "Qualidade"),
    ("楽", "Música/Diversão"), ("音", "Som"), ("色", "Cor"), ("茶", "Chá"), ("肉", "Carne"),
    ("料", "Ingrediente"), ("飯", "Refeição"), ("牛", "Vaca"), ("鳥", "Pássaro"), ("犬", "Cão"),
    ("洋", "Ocidental"), ("和", "Paz/Japonês"), ("夜", "Noite"), ("朝", "Manhã"), ("昼", "Tarde"),
    ("夕", "Tarde/Noite"), ("方", "Direção"), ("晩", "Noite"), ("晴", "Ensolarado"), ("曇", "Nublado"),
    ("風", "Vento"), ("銀", "Prata"), ("員", "Membro"), ("界", "Mundo"), ("旅", "Viagem"),
    ("有", "Ter"), ("無", "Não ter"), ("薬", "Remédio"), ("林", "Bosque"), ("森", "Floresta"),
    ("池", "Lagoa"), ("仕", "Servir"), ("事", "Coisa/Ação")
]

# --- 3. FUNÇÃO DE GERAÇÃO ---
def kanji_to_grid(char, font_path):
    image = Image.new("1", (GRID_SIZE, GRID_SIZE), 0)
    draw = ImageDraw.Draw(image)
    try:
        font = ImageFont.truetype(font_path, 14)
    except IOError:
        print(f"Erro fatal: Fonte '{font_path}' não encontrada.")
        return None

    # Desenha e centra
    draw.text((0, -2), char, font=font, fill=1)
    
    grid = []
    pixels = image.load()
    for y in range(GRID_SIZE):
        row = []
        for x in range(GRID_SIZE):
            row.append(1 if pixels[x, y] > 0 else 0)
        grid.append(row)
    return grid

# --- 4. PROCESSAMENTO ---
output_data = []
print("A gerar novos Kanji...")

count = 0
for k, m in kanji_master_list:
    if k in excluded_kanji:
        continue
        
    grid = kanji_to_grid(k, FONT_PATH)
    if grid:
        output_data.append({
            "kanji": k,
            "meaning": m,
            "grid": grid
        })
        count += 1

# --- 5. SAVING COM FORMATAÇÃO LIMPA ---
# Primeiro fazemos o dump normal
json_str = json.dumps(output_data, ensure_ascii=False, indent=4)

# REGEX MÁGICO: Procura listas de números [0, 0, ...] e remove o whitespace dentro delas
# Pattern: Encontra '[' seguido de (digito ou virgula ou espaço) até ']'
# E substitui por uma versão sem espaços
print("A formatar JSON...")
def collapse_json_array(match):
    # Remove todos os espaços e quebras de linha dentro do match
    return match.group(0).replace(" ", "").replace("\n", "")

# Aplica a regex apenas nas arrays que contêm 0s e 1s
json_str = re.sub(r'\[\s*([01](?:,\s*[01])*)\s*\]', collapse_json_array, json_str)

with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
    f.write(json_str)

print(f"Feito! Adicionados {count} novos Kanji em '{OUTPUT_FILE}'.")
print("A formatação deve estar igual à original: [0,0,0,0...]")
