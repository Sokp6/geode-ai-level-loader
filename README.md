# 🚀 AI Level Loader — Мод для Geode (Geometry Dash 2.2)

Мод для **Geometry Dash 2.2 (Geode SDK)**, который позволяет загружать и тестировать уровни, сгенерированные искусственным интеллектом (ИИ) через JSON-файл, включая блоки, порталы и **триггеры** (Move, Color, Pulse, Alpha, Rotate, Shake, Toggle, Spawn)!

---

## 🌟 Возможности

- 🤖 **Импорт уровней от ИИ из JSON:** Создавайте уровни через промпты для нейросетей и сразу смотрите их в игре.
- ⚡ **Полная поддержка триггеров:**
  - `trigger_move` — движение объектов с группами, временем и 18 типами Easing (`bounce_out`, `ease_in_out`, `elastic` и др.).
  - `trigger_color` — плавная смена цветов фона (`bg`), пола (`g1`, `g2`), линий (`line`), объектов (`obj`).
  - `trigger_pulse` — пульсация цвета для групп или каналов с настройкой Fade In / Hold / Fade Out.
  - `trigger_alpha` — плавное появление/исчезновение объектов через прозрачность (opacity).
  - `trigger_rotate` — вращение объектов вокруг центральной группы.
  - `trigger_shake` — землетрясение / дрожание экрана.
  - `trigger_toggle` — включение/выключение объектов по группе.
  - `trigger_spawn` — цепочки триггеров с задержкой.
- 🧱 **Блоки, шипы, сферы и порталы:**
  - Блоки, полублоки, контуры (`block`, `slab`, `outline`).
  - Все виды шипов (`spike`, `spike_short`, `spike_hanging`).
  - Орбы/сферы (`orb_yellow`, `orb_pink`, `orb_red`, `orb_blue`, `orb_green`, `orb_dash_green`, `orb_black`).
  - Батуты/пады (`pad_yellow`, `pad_pink`, `pad_red`, `pad_blue`).
  - Порталы режимов (`portal_cube`, `portal_ship`, `portal_ball`, `portal_ufo`, `portal_wave`, `portal_swing`).
  - Порталы скоростей (`0.5x`, `1x`, `2x`, `3x`, `4x`).
- 📱 **Кнопка в меню уровня (`EditLevelLayer`):** Кнопка **«AI JSON»** прямо на экране редактирования уровня в GD. Нажали — и уровень сразу загружен!
- 📋 **Кнопка в меню паузы редактора (`EditorPauseLayer`):** Копирует сгенерированные объекты в буфер обмена, чтобы в редакторе можно было нажать **«Paste»** и вставить кусок уровня в любое место.
- 🐍 **Автономный конвертер на Python (`scripts/ai_level_converter.py`):** Работает прямо в Termux на телефоне даже без установки мода!

---

## 📁 Структура проекта

```text
geode-ai-level-loader/
├── mod.json                     # Манифест мода Geode (v4.1.0 / GD 2.2074)
├── CMakeLists.txt               # Конфигурация сборки Geode SDK
├── .github/
│   └── workflows/
│       └── build.yml            # Автоматическая сборка для Android и Windows
├── src/
│   ├── main.cpp                 # Хуки EditLevelLayer и EditorPauseLayer
│   ├── LevelConverter.hpp       # Заголовочный файл конвертера JSON -> GD
│   └── LevelConverter.cpp       # Парсинг JSON и генерация объектов GD
├── examples/
│   ├── demo_level.json          # Пример полноценного уровня от ИИ с триггерами
│   └── trigger_showcase.json    # Демонстрация всех видов триггеров
└── scripts/
    └── ai_level_converter.py    # Python-скрипт для конвертации и работы в Termux
```

---

## 🎮 Как использовать в игре

1. Положите файл с уровнем `ai_level.json` в папку Загрузок на телефоне:  
   `/sdcard/Download/ai_level.json`  
   *(Если файла нет, мод автоматически создаст красивый демо-уровень при первом нажатии!)*
2. Откройте **Geometry Dash** с Geode.
3. Зайдите в **Create** ➔ создайте новый уровень или выберите существующий.
4. Нажмите новую золотую кнопку **«AI JSON»**.
5. Мод загрузит все блоки и триггеры! Нажмите **«Edit»** — уровень готов к игре и тестированию!

---

## 🤖 Как попросить ИИ написать уровень в JSON (Промпт для нейросети)

Вы можете скопировать следующий промт и отправить любой нейросети (ChatGPT, Claude, Gemini и т.д.):

> *"Напиши для меня уровень Geometry Dash 2.2 в формате JSON для мода AI Level Loader. Сделай синхронизированную под музыку секцию куба и волны, добавь платформы с шипами, сферу, смену цвета фона через trigger_color и движение препятствия вверх через trigger_move с плавным easing bounce_out."*

### Пример формата JSON:

```json
{
  "name": "My AI Level",
  "author": "ChatGPT",
  "speed": "1x",
  "bg_color": [15, 20, 50],
  "ground_color": [25, 35, 75],
  "objects": [
    { "type": "portal_speed_normal", "x": 0, "y": 1 },
    { "type": "trigger_color", "x": 2, "y": 1, "channel": "bg", "r": 120, "g": 30, "b": 180, "duration": 1.0 },
    { "type": "block", "x": 5, "y": 1, "groups": [1] },
    { "type": "block", "x": 6, "y": 1, "groups": [1] },
    { "type": "spike", "x": 7, "y": 1 },
    { "type": "orb_yellow", "x": 7, "y": 3 },
    { "type": "block", "x": 8, "y": 1 },
    {
      "type": "trigger_move",
      "x": 3, "y": 2,
      "target_group": 1,
      "move_x_blocks": 0,
      "move_y_blocks": 2,
      "duration": 0.5,
      "easing": "bounce_out"
    },
    {
      "type": "trigger_pulse",
      "x": 6, "y": 3,
      "target_group": 1,
      "r": 255, "g": 255, "b": 0,
      "fade_in": 0.05,
      "hold": 0.2,
      "fade_out": 0.2
    }
  ]
}
```

---

## 🛠️ Сборка мода

### Способ 1: Автоматическая сборка через GitHub (Рекомендуется)
1. Создайте репозиторий на GitHub и загрузите эту папку `geode-ai-level-loader`.
2. В репозитории уже настроен `.github/workflows/build.yml`.
3. GitHub Actions автоматически соберет файл `.geode` сразу для:
   - **Android 64-bit** (arm64-v8a)
   - **Android 32-bit** (armeabi-v7a)
   - **Windows**
4. Скачайте собранный файл `.geode` из раздела **Artifacts** во вкладке **Actions** и установите в Geode!

### Способ 2: Использование прямо в Termux (без компиляции C++)
В папке `scripts/` есть скрипт `ai_level_converter.py`, который работает прямо в Termux:
```bash
# Конвертировать JSON и скопировать в /sdcard/Download/ai_level.json
python3 scripts/ai_level_converter.py examples/demo_level.json -o /sdcard/Download/ai_level.json

# Или скопировать объекты сразу в буфер обмена Android (для вставки в редакторе по кнопке Paste)
python3 scripts/ai_level_converter.py examples/demo_level.json --clipboard
```
