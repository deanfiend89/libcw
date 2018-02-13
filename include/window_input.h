#define CW_EVENT_INPUT_DOWN 0
#define CW_EVENT_INPUT_UP 1
#define CW_EVENT_INPUT_MOVE 2

typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t action;
} inputEvent;

typedef void (*input_event_handler)(int x, int y, int action);

#ifdef __cplusplus
extern "C" {
#endif

void input_create_queue();
int input_event_available();
inline void input_event_write(uint8_t x, uint8_t y, uint8_t action);
void input_event_read(uint8_t* x, uint8_t* y, uint8_t* action);

#ifdef __cplusplus
}
#endif
