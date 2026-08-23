#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>

void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b) {
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (v * (255 - s)) / 255;
    uint8_t q = (v * (255 - (s * f) / 255)) / 255;
    uint8_t t = (v * (255 - (s * (255 - f)) / 255)) / 255;

    switch((h / 60) % 6) {
        case 0: *r = v; *g = t; *b = p; break;
        case 1: *r = q; *g = v; *b = p; break;
        case 2: *r = p; *g = v; *b = t; break;
        case 3: *r = p; *g = q; *b = v; break;
        case 4: *r = t; *g = p; *b = v; break;
        case 5: *r = v; *g = p; *b = q; break;
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 10, 20, "SuperLED RGB");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 10, 40, "Arc-en-ciel actif...");
    canvas_draw_str(canvas, 10, 55, "Quitter: Retour [Bleu fixe]");
}

int32_t superledrgb_app(void* p) {
    UNUSED(p);
    
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, NULL);
    
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    uint16_t hue = 0;
    uint8_t r = 0, g = 0, b = 0;
    bool running = true;

    furi_hal_light_sequence_stop();

    while(running) {
        InputEvent event;
        if(furi_message_queue_get(event_queue, &event, furi_ms_to_ticks(20)) == FuriStatusOk) {
            if(event.type == InputTypeShort && event.key == InputKeyBack) {
                running = false;
            }
        }

        hsv_to_rgb(hue, 255, 255, &r, &g, &b);
        furi_hal_light_set(LightRed, r);
        furi_hal_light_set(LightGreen, g);
        furi_hal_light_set(LightBlue, b);

        hue = (hue + 2) % 360;
        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);

    furi_hal_light_set(LightRed, 0);
    furi_hal_light_set(LightGreen, 0);
    furi_hal_light_set(LightBlue, 255); 

    return 0;
}
