#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>

#define TAG "FlipperZeroAlarm"
#define DEFAULT_PASSWORD {InputKeyUp, InputKeyUp, InputKeyDown, InputKeyDown}

typedef struct {
    uint32_t alarm_time;
    uint8_t password[4];
    bool alarm_active;
    bool alarm_triggered;
} AlarmState;

static void draw_callback(Canvas* canvas, void* ctx) {
    AlarmState* state = ctx;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    
    if(state->alarm_triggered) {
        canvas_draw_str(canvas, 10, 30, "ALARM TRIGGERED!");
        canvas_draw_str(canvas, 10, 50, "Enter Password:");
    } else {
        canvas_draw_str(canvas, 10, 30, "Alarm Status:");
        canvas_draw_str(canvas, 10, 50, state->alarm_active ? "ACTIVE" : "INACTIVE");
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t flipperzero_alarm_app(void* p) {
    UNUSED(p);
    
    AlarmState state = {
        .alarm_time = 300, // 5 minutes
        .password = DEFAULT_PASSWORD,
        .alarm_active = false,
        .alarm_triggered = false
    };
    
    ViewPort* view_port = view_port_alloc();
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    
    view_port_draw_callback_set(view_port, draw_callback, &state);
    view_port_input_callback_set(view_port, input_callback, event_queue);
    
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    
    InputEvent event;
    while(1) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypePress) {
                if(event.key == InputKeyBack) break;
                
                if(state.alarm_triggered) {
                    // Password check logic here
                } else {
                    if(event.key == InputKeyOk) {
                        state.alarm_active = !state.alarm_active;
                    }
                }
            }
        }
        
        if(state.alarm_active && !state.alarm_triggered) {
            if(state.alarm_time > 0) {
                state.alarm_time--;
            } else {
                state.alarm_triggered = true;
                notification_message(notifications, &sequence_alarm);
            }
        }
        
        view_port_update(view_port);
    }
    
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    return 0;
}
