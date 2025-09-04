#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "lvgl/lvgl.h"
#include "game_2048.h"

/* Configuration for board size and UI */
#define G2048_SIZE 4

/* Colors tuned for readability; use default theme palettes to avoid heavy assets */
static inline lv_color_t tile_color_for_value(uint32_t v) {
    switch(v) {
        case 0:   return lv_palette_lighten(LV_PALETTE_BLUE_GREY, 4);
        case 2:   return lv_palette_lighten(LV_PALETTE_AMBER, 4);
        case 4:   return lv_palette_lighten(LV_PALETTE_AMBER, 3);
        case 8:   return lv_palette_main(LV_PALETTE_AMBER);
        case 16:  return lv_palette_main(LV_PALETTE_DEEP_ORANGE);
        case 32:  return lv_palette_darken(LV_PALETTE_DEEP_ORANGE, 1);
        case 64:  return lv_palette_darken(LV_PALETTE_DEEP_ORANGE, 2);
        case 128: return lv_palette_main(LV_PALETTE_GREEN);
        case 256: return lv_palette_darken(LV_PALETTE_GREEN, 1);
        case 512: return lv_palette_darken(LV_PALETTE_GREEN, 2);
        case 1024:return lv_palette_main(LV_PALETTE_BLUE);
        default:  return lv_palette_darken(LV_PALETTE_BLUE, 1);
    }
}

typedef struct {
    uint16_t cols;
    uint16_t rows;
    uint32_t cells[G2048_SIZE * G2048_SIZE];
    uint32_t score;
    bool moved_last;
    bool won;
    /* UI references */
    lv_obj_t * root;
    lv_obj_t * header;
    lv_obj_t * grid;
    lv_obj_t * score_label;
    lv_obj_t * status_label;
    lv_obj_t * reset_btn;
    lv_obj_t * tiles[G2048_SIZE * G2048_SIZE];
    /* gesture state */
    lv_point_t touch_start;
    bool touch_active;
} g2048_t;

static g2048_t * G = NULL;

static void g2048_reset_board(g2048_t *g);
static void g2048_spawn_random(g2048_t *g);
static bool g2048_can_move(const g2048_t *g);
static bool g2048_move(g2048_t *g, lv_dir_t dir);
static void g2048_update_ui(g2048_t *g, bool animate);
static void g2048_on_reset(lv_event_t * e);
static void g2048_on_gesture(lv_event_t * e);
static void g2048_on_pointer(lv_event_t * e);
static void g2048_on_size_changed(lv_event_t * e);

static inline uint32_t * cell(g2048_t *g, uint16_t r, uint16_t c) {
    return &g->cells[r * g->cols + c];
}

static void g2048_add_random_tile(g2048_t *g) {
    /* Collect empty cells */
    uint16_t empty_positions[G2048_SIZE * G2048_SIZE];
    uint16_t empty_cnt = 0;
    for(uint16_t r=0; r<g->rows; ++r) {
        for(uint16_t c=0; c<g->cols; ++c) {
            if(*cell(g,r,c) == 0) empty_positions[empty_cnt++] = (uint16_t)(r * g->cols + c);
        }
    }
    if(empty_cnt == 0) return;
    uint16_t pick_idx = (uint16_t)lv_rand(0, empty_cnt - 1);
    uint16_t pos = empty_positions[pick_idx];
    uint16_t pr = (uint16_t)(pos / g->cols);
    uint16_t pc = (uint16_t)(pos % g->cols);
    *cell(g, pr, pc) = (lv_rand(0, 9) == 0) ? 4 : 2; /* 10% for 4 */
}

static void g2048_reset_board(g2048_t *g) {
    g->score = 0;
    g->won = false;
    memset(g->cells, 0, sizeof(g->cells));
    g2048_add_random_tile(g);
    g2048_add_random_tile(g);
}

static void g2048_spawn_random(g2048_t *g) {
    g2048_add_random_tile(g);
}

static bool g2048_has_move_available(const g2048_t *g) {
    /* Empty cell? */
    for(uint16_t i=0;i<g->rows*g->cols;i++) if(g->cells[i]==0) return true;
    /* Adjacent equal? */
    for(uint16_t r=0;r<g->rows;r++) {
        for(uint16_t c=0;c<g->cols;c++) {
            uint32_t v = g->cells[r*g->cols+c];
            if(c+1<g->cols && g->cells[r*g->cols+c+1]==v) return true;
            if(r+1<g->rows && g->cells[(r+1)*g->cols+c]==v) return true;
        }
    }
    return false;
}

static bool g2048_can_move(const g2048_t *g) {
    return g2048_has_move_available(g);
}

static bool compress_line(uint32_t *line, uint16_t n, uint32_t *score_gain) {
    bool moved = false;
    /* Shift non-zero left */
    uint16_t last = 0;
    for(uint16_t i=0;i<n;i++) if(line[i]!=0) {
        if(i!=last) { line[last]=line[i]; line[i]=0; moved=true; }
        last++;
    }
    /* Merge */
    for(uint16_t i=0;i+1<n;i++) {
        if(line[i]!=0 && line[i]==line[i+1]) {
            line[i]*=2; line[i+1]=0; *score_gain += line[i]; moved = true; i++; /* skip next */
        }
    }
    /* Shift again */
    last = 0;
    for(uint16_t i=0;i<n;i++) if(line[i]!=0) {
        if(i!=last) { line[last]=line[i]; line[i]=0; moved=true; }
        last++;
    }
    return moved;
}

static bool g2048_move(g2048_t *g, lv_dir_t dir) {
    bool moved = false;
    uint32_t score_gain = 0;

    uint32_t tmp[G2048_SIZE];

    if(dir == LV_DIR_LEFT || dir == LV_DIR_RIGHT) {
        for(uint16_t r=0;r<g->rows;r++) {
            for(uint16_t c=0;c<g->cols;c++) tmp[c] = *cell(g,r,c);
            if(dir == LV_DIR_RIGHT) {
                /* reverse */
                for(uint16_t i=0;i<g->cols/2;i++) { uint32_t t=tmp[i]; tmp[i]=tmp[g->cols-1-i]; tmp[g->cols-1-i]=t; }
            }
            (void)compress_line(tmp, g->cols, &score_gain);
            if(dir == LV_DIR_RIGHT) {
                for(uint16_t i=0;i<g->cols/2;i++) { uint32_t t=tmp[i]; tmp[i]=tmp[g->cols-1-i]; tmp[g->cols-1-i]=t; }
            }
            for(uint16_t c=0;c<g->cols;c++) {
                if(*cell(g,r,c) != tmp[c]) { *cell(g,r,c) = tmp[c]; moved = true; }
            }
        }
    } else if(dir == LV_DIR_TOP || dir == LV_DIR_BOTTOM) {
        for(uint16_t c=0;c<g->cols;c++) {
            for(uint16_t r=0;r<g->rows;r++) tmp[r] = *cell(g,r,c);
            if(dir == LV_DIR_BOTTOM) {
                for(uint16_t i=0;i<g->rows/2;i++) { uint32_t t=tmp[i]; tmp[i]=tmp[g->rows-1-i]; tmp[g->rows-1-i]=t; }
            }
            (void)compress_line(tmp, g->rows, &score_gain);
            if(dir == LV_DIR_BOTTOM) {
                for(uint16_t i=0;i<g->rows/2;i++) { uint32_t t=tmp[i]; tmp[i]=tmp[g->rows-1-i]; tmp[g->rows-1-i]=t; }
            }
            for(uint16_t r=0;r<g->rows;r++) {
                if(*cell(g,r,c) != tmp[r]) { *cell(g,r,c) = tmp[r]; moved = true; }
            }
        }
    }

    if(moved) {
        g->score += score_gain;
        for(uint16_t i=0;i<g->rows*g->cols;i++) if(g->cells[i]==2048) g->won = true;
    }
    return moved;
}

static void animate_tile_move(lv_obj_t * obj, lv_coord_t x, lv_coord_t y) {
    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, lv_obj_get_x(obj), x);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_time(&a, 90);
    lv_anim_start(&a);

    lv_anim_t b; lv_anim_init(&b);
    lv_anim_set_var(&b, obj);
    lv_anim_set_values(&b, lv_obj_get_y(obj), y);
    lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_time(&b, 90);
    lv_anim_start(&b);
}

static void anim_exec_set_zoom(void * var, int32_t v) {
    lv_obj_set_style_transform_zoom((lv_obj_t *)var, (int32_t)v, 0);
}

static void animate_tile_scale(lv_obj_t * obj) {
    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    int32_t z0 = LV_SCALE_NONE;                   /* 256 */
    int32_t z1 = (LV_SCALE_NONE * 108) / 100;     /* ~108% */
    lv_anim_set_values(&a, z0, z1);
    lv_anim_set_exec_cb(&a, anim_exec_set_zoom);
    lv_anim_set_time(&a, 90);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_start(&a);
}

static void set_tile_visual(lv_obj_t * tile, uint32_t v) {
    lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(tile, tile_color_for_value(v), 0);
    lv_obj_t * label = lv_obj_get_child(tile, 0);
    if(label == NULL) {
        label = lv_label_create(tile);
        lv_obj_center(label);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    }
    if(v == 0) lv_obj_add_flag(tile, LV_OBJ_FLAG_HIDDEN);
    else {
        lv_obj_clear_flag(tile, LV_OBJ_FLAG_HIDDEN);
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%lu", (unsigned long)v);
        lv_label_set_text(label, buf);
    }
}

static void g2048_update_ui(g2048_t *g, bool animate) {
    /* Update dynamic texts first; status visibility impacts available space */
    char sbuf[32];
    lv_snprintf(sbuf, sizeof(sbuf), "Score: %lu", (unsigned long)g->score);
    lv_label_set_text(g->score_label, sbuf);

    if(g->won) {
        lv_label_set_text(g->status_label, "You reached 2048!");
        lv_obj_clear_flag(g->status_label, LV_OBJ_FLAG_HIDDEN);
    } else if(!g2048_can_move(g)) {
        lv_label_set_text(g->status_label, "Game Over");
        lv_obj_clear_flag(g->status_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g->status_label, LV_OBJ_FLAG_HIDDEN);
    }

    /* Compute available space accounting for header/status/paddings, keep square board */
    lv_obj_update_layout(g->root);

    lv_coord_t sw = lv_display_get_horizontal_resolution(NULL);
    lv_coord_t sh = lv_display_get_vertical_resolution(NULL);

    lv_coord_t pad_l = lv_obj_get_style_pad_left(g->root, 0);
    lv_coord_t pad_r = lv_obj_get_style_pad_right(g->root, 0);
    lv_coord_t pad_t = lv_obj_get_style_pad_top(g->root, 0);
    lv_coord_t pad_b = lv_obj_get_style_pad_bottom(g->root, 0);
    lv_coord_t row_gap = lv_obj_get_style_pad_row(g->root, 0);

    lv_coord_t header_h = lv_obj_get_height(g->header);
    lv_coord_t status_h = lv_obj_has_flag(g->status_label, LV_OBJ_FLAG_HIDDEN) ? 0 : lv_obj_get_height(g->status_label);

    lv_coord_t avail_w = sw - pad_l - pad_r;
    lv_coord_t avail_h = sh - pad_t - pad_b - header_h - status_h - row_gap * 2;
    if(avail_w < 0) avail_w = 0;
    if(avail_h < 0) avail_h = 0;

    lv_coord_t board = (avail_w < avail_h) ? avail_w : avail_h;
    if(board < 160) board = 160;

    lv_obj_set_size(g->grid, board, board);

    /* Gap scaled with board size within sensible range */
    lv_coord_t gap = board / 80; /* ~5-8px typically */
    if(gap < 4) gap = 4;
    if(gap > 14) gap = 14;

    lv_coord_t cell_size = (board - gap * (g->cols + 1)) / g->cols;
    
    for(uint16_t r=0;r<g->rows;r++) {
        for(uint16_t c=0;c<g->cols;c++) {
            lv_obj_t * t = g->tiles[r*g->cols+c];
            lv_coord_t x = gap + c * (cell_size + gap);
            lv_coord_t y = gap + r * (cell_size + gap);
            lv_obj_set_size(t, cell_size, cell_size);
            if(animate) animate_tile_move(t, x, y); else lv_obj_set_pos(t, x, y);
            set_tile_visual(t, *cell(g,r,c));
        }
    }
}

static void g2048_on_reset(lv_event_t * e) {
    LV_UNUSED(e);
    if(!G) return;
    g2048_reset_board(G);
    g2048_update_ui(G, false);
}

static void g2048_on_gesture(lv_event_t * e) {
    /* If LV_USE_GESTURE_RECOGNITION is enabled we could use gesture type/dir.
       For portability, still handle pointer drag as fallback. */
#if LV_USE_GESTURE_RECOGNITION
    lv_indev_t * indev = lv_indev_active();
    if(!indev) return;
    lv_dir_t dir = lv_indev_get_gesture_dir(indev);
    if(dir == 0) return;
    if(g2048_move(G, dir)) {
        g2048_spawn_random(G);
        g2048_update_ui(G, true);
        if(!g2048_can_move(G)) g2048_update_ui(G, false);
    }
#else
    LV_UNUSED(e);
#endif
}

static void g2048_on_pointer(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    LV_UNUSED(obj);
    lv_indev_t * indev = lv_indev_get_act();
    if(!indev) return;
    lv_point_t p; lv_indev_get_point(indev, &p);

    if(code == LV_EVENT_PRESSED) {
        G->touch_active = true;
        G->touch_start = p;
    } else if(code == LV_EVENT_RELEASED && G->touch_active) {
        G->touch_active = false;
        int dx = p.x - G->touch_start.x;
        int dy = p.y - G->touch_start.y;
        int adx = dx>0?dx:-dx;
        int ady = dy>0?dy:-dy;
        const int threshold = 20; /* px */
        if(adx < threshold && ady < threshold) return;
        lv_dir_t dir;
        if(adx > ady) dir = dx>0?LV_DIR_RIGHT:LV_DIR_LEFT;
        else dir = dy>0?LV_DIR_BOTTOM:LV_DIR_TOP;
        if(g2048_move(G, dir)) {
            g2048_spawn_random(G);
            g2048_update_ui(G, true);
            /* highlight merged tiles via scale animation */
            for(uint16_t r=0;r<G->rows;r++) for(uint16_t c=0;c<G->cols;c++) if(*cell(G,r,c)!=0) animate_tile_scale(G->tiles[r*G->cols+c]);
        }
        if(!g2048_can_move(G)) g2048_update_ui(G, false);
    }
}

void game_2048_start(void) {
    if(G) {
        /* already running */
        g2048_update_ui(G, false);
        return;
    }

    g2048_t * g = (g2048_t *)lv_malloc_zeroed(sizeof(g2048_t));
    LV_ASSERT_MALLOC(g);
    g->cols = G2048_SIZE;
    g->rows = G2048_SIZE;
    G = g;

    lv_obj_t * scr = lv_screen_active();

    /* Root container */
    g->root = lv_obj_create(scr);
    lv_obj_remove_style_all(g->root);
    lv_obj_set_size(g->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(g->root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g->root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(g->root, LV_DIR_VER);
    lv_obj_set_style_pad_all(g->root, 8, 0);
    lv_obj_set_style_pad_row(g->root, 8, 0);

    /* Header: title + score + reset */
    lv_obj_t * header = lv_obj_create(g->root);
    lv_obj_remove_style_all(header);
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(header, 8, 0);
    lv_obj_set_style_pad_all(header, 4, 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
    g->header = header;

    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);

    g->score_label = lv_label_create(header);
    lv_label_set_text(g->score_label, "Score: 0");
    lv_obj_add_flag(g->score_label, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    g->reset_btn = lv_button_create(header);
    lv_obj_t * rlab = lv_label_create(g->reset_btn);
    lv_label_set_text(rlab, "Reset");
    lv_obj_center(rlab);
    lv_obj_add_event_cb(g->reset_btn, g2048_on_reset, LV_EVENT_CLICKED, NULL);

    g->status_label = lv_label_create(g->root);
    lv_label_set_text(g->status_label, "");
    lv_obj_add_flag(g->status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_color(g->status_label, lv_palette_main(LV_PALETTE_RED), 0);

    /* Board container */
    g->grid = lv_obj_create(g->root);
    lv_obj_remove_style_all(g->grid);
    lv_obj_set_style_bg_color(g->grid, lv_palette_lighten(LV_PALETTE_BLUE_GREY, 3), 0);
    lv_obj_set_style_bg_opa(g->grid, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g->grid, 8, 0);

    /* React to container size changes to keep layout responsive */
    lv_obj_add_event_cb(g->root, g2048_on_size_changed, LV_EVENT_SIZE_CHANGED, NULL);
    lv_obj_add_event_cb(g->grid, g2048_on_size_changed, LV_EVENT_SIZE_CHANGED, NULL);

    /* Create tiles */
    for(uint16_t r=0;r<g->rows;r++) {
        for(uint16_t c=0;c<g->cols;c++) {
            lv_obj_t * tile = lv_obj_create(g->grid);
            lv_obj_remove_style_all(tile);
            lv_obj_set_style_radius(tile, 6, 0);
            lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
            lv_obj_set_style_shadow_width(tile, 8, 0);
            lv_obj_set_style_shadow_opa(tile, LV_OPA_20, 0);
            lv_obj_set_style_shadow_ofs_y(tile, 2, 0);
            g->tiles[r*g->cols+c] = tile;
        }
    }

    /* Input handling: gesture + pointer swipe */
    lv_obj_add_event_cb(g->grid, g2048_on_gesture, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(g->grid, g2048_on_pointer, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(g->grid, g2048_on_pointer, LV_EVENT_RELEASED, NULL);

    /* Initialize state */
    g2048_reset_board(g);
    g2048_update_ui(g, false);
}

static void g2048_on_size_changed(lv_event_t * e) {
    LV_UNUSED(e);
    if(!G) return;
    g2048_update_ui(G, false);
}
