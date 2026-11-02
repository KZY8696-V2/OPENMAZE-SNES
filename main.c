#include <snes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* --- SNES / Super Famicom Donanım Register Haritası --- */
#define REG_INIDISP   (*(volatile uint8_t*)0x2100)
#define REG_BG1SC     (*(volatile uint8_t*)0x2107)
#define REG_BG12NBA   (*(volatile uint8_t*)0x210B)
#define REG_TM        (*(volatile uint8_t*)0x212C)
#define REG_CGADD     (*(volatile uint8_t*)0x2121)
#define REG_CGDATA    (*(volatile uint16_t*)0x2122)
#define REG_BG1HOFS   (*(volatile uint16_t*)0x210D)
#define REG_BG1VOFS   (*(volatile uint16_t*)0x210E)

/* --- SRAM Batarya Destekli Kayıt Alanı --- */
#define SRAM_BASE_ADDR ((volatile uint8_t*)0x700000)
#define SAVE_MAGIC_KEY 0x4B475356 // "KGSV" (KGAMER SAVE)

#define MAX_WIDTH     64
#define MAX_HEIGHT    64
#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 224
#define TILE_SIZE     8

/* --- Oyun Durumları ve Modlar --- */
typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_SETTINGS
} GameState;

typedef enum {
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD,
    DIFFICULTY_CUSTOM
} Difficulty;

typedef struct {
    bool ascii_mode;    // true: ASCII karakterler, false: Grafik Tile/Pixel Art
    bool no_color_mode; // true: Siyah-Beyaz, false: Renkli
} RenderMode;

typedef struct {
    uint16_t wall_color;
    uint16_t path_color;
    uint16_t player_color;
    bool custom_colors_enabled;
} ColorSettings;

/* SRAM'e Kalıcı Olarak Kaydedilecek Ayar Yapısı */
typedef struct {
    uint32_t magic_key;
    Difficulty difficulty;
    int custom_width;
    int custom_height;
    bool is_timed;
    uint32_t total_time;
    RenderMode render;
    ColorSettings colors;
} PersistentSettings;

typedef struct {
    GameState state;
    int width;
    int height;
    int player_x;
    int player_y;
    int exit_x;
    int exit_y;
    uint32_t time_remaining;
    uint32_t total_time;
    bool is_timed;
    Difficulty difficulty;
    RenderMode render;
    int current_round;
    ColorSettings colors;
    int scroll_x;
    int scroll_y;
    int menu_selection;     // Ana menü ve pause menüsü imleci
    int settings_selection; // Ayarlar menüsü imleci (0: Zorluk, 1: ASCII Mod, 2: Siyah-Beyaz, 3: Süre, 4: Geri)
} MazeGame;

/* --- Global Değişkenler --- */
uint8_t maze_grid[MAX_HEIGHT][MAX_WIDTH];
MazeGame game_instance;

/* --- 16x16 Pixel Art Kertenkele (Lizard) Karakter Grafiği --- */
const uint8_t LIZARD_SPRITE_16x16[128] = {
    0x00, 0x00, 0x03, 0xC0, 0x03, 0xF0, 0x0F, 0xFC,
    0x3F, 0xFF, 0x7F, 0xFE, 0x7F, 0xFE, 0x3F, 0xFC,
    0x1F, 0xF8, 0x0F, 0xF0, 0x1B, 0xD8, 0x36, 0x6C,
    0x7F, 0xFE, 0xFF, 0xFF, 0xAA, 0xAA, 0x55, 0x55,
    0x00, 0x00, 0x01, 0x80, 0x03, 0xC0, 0x07, 0xE0,
    0x0F, 0xF0, 0x1F, 0xF8, 0x1F, 0xF8, 0x0F, 0xF0,
    0x07, 0xE0, 0x03, 0xC0, 0x0D, 0xB0, 0x1A, 0x58,
    0x3F, 0xFC, 0x7F, 0xFE, 0x55, 0x55, 0xAA, 0xAA
};

/* --- Rastgele Sayı Üreteci (LFSR Tabanlı) --- */
static uint32_t snes_rng_seed = 0x87654321;
uint32_t snes_rand(void) {
    snes_rng_seed = (snes_rng_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return snes_rng_seed;
}

uint16_t snes_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)((b & 0x1F) << 10) | ((g & 0x1F) << 5) | (r & 0x1F);
}

/* --- Ayarları SRAM Bataryasına Kaydetme --- */
void saveSettingsToSram(MazeGame *game) {
    PersistentSettings *sram_data = (PersistentSettings*)SRAM_BASE_ADDR;
    sram_data->magic_key = SAVE_MAGIC_KEY;
    sram_data->difficulty = game->difficulty;
    sram_data->custom_width = game->width;
    sram_data->custom_height = game->height;
    sram_data->is_timed = game->is_timed;
    sram_data->total_time = game->total_time;
    sram_data->render = game->render;
    sram_data->colors = game->colors;
}

/* --- SRAM'den Ayarları Yükleme --- */
bool loadSettingsFromSram(MazeGame *game) {
    PersistentSettings *sram_data = (PersistentSettings*)SRAM_BASE_ADDR;
    if (sram_data->magic_key == SAVE_MAGIC_KEY) {
        game->difficulty = sram_data->difficulty;
        game->width = sram_data->custom_width;
        game->height = sram_data->custom_height;
        game->is_timed = sram_data->is_timed;
        game->total_time = sram_data->total_time;
        game->render = sram_data->render;
        game->colors = sram_data->colors;
        return true;
    }
    return false;
}

/* --- Labirentin Ekranda Tam Ortalanması --- */
void updateMazeCentering(MazeGame *game) {
    int total_w = game->width * TILE_SIZE;
    int total_h = game->height * TILE_SIZE;

    int target_x = (total_w > SCREEN_WIDTH) ? 0 : (SCREEN_WIDTH - total_w) / 2;
    int target_y = (total_h > SCREEN_HEIGHT) ? 0 : (SCREEN_HEIGHT - total_h) / 2;

    game->scroll_x = -target_x;
    game->scroll_y = -target_y;

    REG_BG1HOFS = (uint16_t)game->scroll_x;
    REG_BG1VOFS = (uint16_t)game->scroll_y;
}

/* --- Zorluk Derecesine Göre Boyut ve Süre Ayarı --- */
void applyDifficulty(MazeGame *game) {
    switch (game->difficulty) {
        case DIFFICULTY_EASY:
            game->width = 15;
            game->height = 15;
            game->total_time = 120;
            break;
        case DIFFICULTY_MEDIUM:
            game->width = 31;
            game->height = 31;
            game->total_time = 90;
            break;
        case DIFFICULTY_HARD:
            game->width = 63;
            game->height = 63;
            game->total_time = 60;
            break;
        case DIFFICULTY_CUSTOM:
            // Custom modda mevcut boyutlar korunur
            break;
    }
}

/* --- Renk ve Render Modları Yönetimi (Menüler ve Labirent Dahil) --- */
void applyRenderAndColors(MazeGame *game) {
    if (game->render.no_color_mode) {
        if (game->render.ascii_mode) {
            game->colors.wall_color = snes_rgb(0, 0, 0);   // Siyah
            game->colors.path_color = snes_rgb(31, 31, 31); // Beyaz
        } else {
            game->colors.wall_color = snes_rgb(4, 4, 4);
            game->colors.path_color = snes_rgb(28, 28, 28);
        }
        game->colors.player_color = snes_rgb(31, 31, 31);
    } else {
        if (!game->colors.custom_colors_enabled) {
            uint8_t rw, gw, bw, rp, gp, bp;
            do {
                rw = (snes_rand() % 26) + 3;
                gw = (snes_rand() % 26) + 3;
                bw = (snes_rand() % 26) + 3;
                rp = (snes_rand() % 26) + 3;
                gp = (snes_rand() % 26) + 3;
                bp = (snes_rand() % 26) + 3;
            } while ((abs((int)rw - rp) + abs((int)gw - gp) + abs((int)bw - bp)) < 25);

            game->colors.wall_color = snes_rgb(rw, gw, bw);
            game->colors.path_color = snes_rgb(rp, gp, bp);
        }
        game->colors.player_color = snes_rgb(31, 31, 0); // Kertenkele Rengi
    }

    REG_CGADD = 0x01; REG_CGDATA = game->colors.wall_color;
    REG_CGADD = 0x02; REG_CGDATA = game->colors.path_color;
    REG_CGADD = 0x03; REG_CGDATA = game->colors.player_color;
}

/* --- Garantili Çözülebilir Labirent Üretimi (DFS Algoritması) --- */
void generateMaze(MazeGame *game) {
    int w = game->width;
    int h = game->height;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            maze_grid[y][x] = 1; // Duvar
        }
    }

    int cx = 1, cy = 1;
    maze_grid[cy][cx] = 0;

    int steps = (w * h) * 2;
    for (int i = 0; i < steps; i++) {
        int dx[] = {0, 0, 2, -2};
        int dy[] = {2, -2, 0, 0};
        int r = snes_rand() % 4;

        int nx = cx + dx[r];
        int ny = cy + dy[r];

        if (nx > 0 && nx < w - 1 && ny > 0 && ny < h - 1) {
            if (maze_grid[ny][nx] == 1) {
                maze_grid[cy + dy[r] / 2][cx + dx[r] / 2] = 0;
                maze_grid[ny][nx] = 0;
                cx = nx;
                cy = ny;
            }
        }
    }

    game->player_x = 1;
    game->player_y = 1;
    maze_grid[game->player_y][game->player_x] = 0;

    game->exit_x = w - 2;
    game->exit_y = h - 2;
    maze_grid[game->exit_y][game->exit_x] = 2; // Çıkış
}

/* --- Oyunu Başlat veya Sonraki Raunda Geç --- */
void startGameRound(MazeGame *game) {
    REG_INIDISP = 0x80; // Ekran Karartma
    
    applyDifficulty(game);
    game->time_remaining = game->total_time;
    applyRenderAndColors(game);
    generateMaze(game);
    updateMazeCentering(game);
    
    game->state = STATE_PLAYING;
    REG_INIDISP = 0x0F; // Ekran Açma
}

/* --- Ana Menüye Dönüş --- */
void returnToMainMenu(MazeGame *game) {
    REG_INIDISP = 0x80;
    applyRenderAndColors(game);
    game->state = STATE_MAIN_MENU;
    game->menu_selection = 0;
    REG_INIDISP = 0x0F;
}

/* --- Kontrolcü Girdilerini İşleme ve Tüm Menülerde D-Pad Desteği --- */
void handleInput(MazeGame *game) {
    uint16_t keys = padsCurrent(0); // PVSnesLib kontrolcü okuma fonksiyonu

    switch (game->state) {
        case STATE_MAIN_MENU:
            if (keys & KEY_UP) {
                if (game->menu_selection > 0) game->menu_selection--;
            }
            if (keys & KEY_DOWN) {
                if (game->menu_selection < 1) game->menu_selection++; // 0: Oyna, 1: Ayarlar
            }
            if (keys & KEY_A) {
                if (game->menu_selection == 0) {
                    startGameRound(game);
                } else if (game->menu_selection == 1) {
                    game->state = STATE_SETTINGS;
                    game->settings_selection = 0;
                }
            }
            break;

        case STATE_SETTINGS:
            // Ayarlar menüsünde D-Pad YUKARI / AŞAĞI ile seçenekler arası gezinme
            if (keys & KEY_UP) {
                if (game->settings_selection > 0) game->settings_selection--;
            }
            if (keys & KEY_DOWN) {
                if (game->settings_selection < 4) game->settings_selection++;
            }

            // Ayarlar menüsünde D-Pad SOL / SAĞ ile değerleri değiştirme
            if (keys & KEY_LEFT || keys & KEY_RIGHT) {
                if (game->settings_selection == 0) {
                    // Zorluk Değiştirme
                    if (game->difficulty > 0) game->difficulty--;
                    else game->difficulty = DIFFICULTY_CUSTOM;
                } else if (game->settings_selection == 1) {
                    // ASCII Mod Aç/Kapat
                    game->render.ascii_mode = !game->render.ascii_mode;
                } else if (game->settings_selection == 2) {
                    // Siyah-Beyaz (No Color) Mod Aç/Kapat
                    game->render.no_color_mode = !game->render.no_color_mode;
                } else if (game->settings_selection == 3) {
                    // Süreli / Süresiz Değiştirme
                    game->is_timed = !game->is_timed;
                }
            }

            // A tuşu veya B tuşu ile ayarları kaydet ve ana menüye dön
            if (keys & KEY_A || keys & KEY_B) {
                if (game->settings_selection == 4 || keys & KEY_B) {
                    saveSettingsToSram(game);
                    returnToMainMenu(game);
                }
            }
            break;

        case STATE_PLAYING:
            if (keys & KEY_START) {
                game->state = STATE_PAUSED;
                game->menu_selection = 0;
                break;
            }

            // D-Pad ile Labirent İçinde Gezinme
            int new_x = game->player_x;
            int new_y = game->player_y;

            if (keys & KEY_UP)    new_y--;
            if (keys & KEY_DOWN)  new_y++;
            if (keys & KEY_LEFT)  new_x--;
            if (keys & KEY_RIGHT) new_x++;

            if (new_x >= 0 && new_x < game->width && new_y >= 0 && new_y < game->height) {
                if (maze_grid[new_y][new_x] != 1) {
                    game->player_x = new_x;
                    game->player_y = new_y;

                    // Büyük labirentlerde kamera kaydırma (Scrolling)
                    int total_w = game->width * TILE_SIZE;
                    int total_h = game->height * TILE_SIZE;
                    if (total_w > SCREEN_WIDTH || total_h > SCREEN_HEIGHT) {
                        int cam_x = (game->player_x * TILE_SIZE) - (SCREEN_WIDTH / 2);
                        int cam_y = (game->player_y * TILE_SIZE) - (SCREEN_HEIGHT / 2);
                        
                        if (cam_x < 0) cam_x = 0;
                        if (cam_y < 0) cam_y = 0;
                        
                        REG_BG1HOFS = (uint16_t)cam_x;
                        REG_BG1VOFS = (uint16_t)cam_y;
                    }

                    // Çıkışa ulaşıldı mı?
                    if (game->player_x == game->exit_x && game->player_y == game->exit_y) {
                        game->current_round++;
                        startGameRound(game);
                    }
                }
            }
            break;

        case STATE_PAUSED:
            if (keys & KEY_UP || keys & KEY_DOWN) {
                game->menu_selection = 1 - game->menu_selection;
            }
            if (keys & KEY_A) {
                if (game->menu_selection == 0) {
                    game->state = STATE_PLAYING;
                } else {
                    returnToMainMenu(game);
                }
            }
            break;
    }
}

/* --- Oyun Çevrim Güncellemesi --- */
void updateGame(MazeGame *game) {
    if (game->state == STATE_PLAYING && game->is_timed) {
        if (game->time_remaining > 0) {
            game->time_remaining--;
        } else {
            returnToMainMenu(game);
        }
    }
}

/* --- Ana Giriş Noktası (Main) --- */
int main(void) {
    consoleInit();
    
    game_instance.current_round = 1;

    // SRAM'den Ayarları Yükle veya Varsayılanları Ata
    if (!loadSettingsFromSram(&game_instance)) {
        game_instance.difficulty = DIFFICULTY_MEDIUM;
        game_instance.width = 31;
        game_instance.height = 31;
        game_instance.is_timed = true;
        game_instance.total_time = 90;
        game_instance.render.ascii_mode = false;
        game_instance.render.no_color_mode = false;
        game_instance.colors.custom_colors_enabled = false;
        saveSettingsToSram(&game_instance);
    }

    returnToMainMenu(&game_instance);

    // Ana Oyun Döngüsü
    while (1) {
        handleInput(&game_instance);
        updateGame(&game_instance);
        WaitForVBlank();
    }

    return 0;
}
