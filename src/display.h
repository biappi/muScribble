void display_init(void);
void display_goto_line_column(int line, int column);
void display_send_empty_screen(void);
void display_send_character(char c);
void display_send_string(const char *string);

typedef enum {
    display_selection_1,
    display_selection_2,
    display_selection_3,
    display_selection_4,
    display_selection_5,
    display_selection_6,
    display_selection_7,
    display_selection_8,
    display_selection_all,
    display_selection_none,
} display_selection_t;

void display_select(display_selection_t selection);

void display_transport_reset(void);
void display_transport_set_control(void);
void display_transport_set_data(void);
void display_transport_write(char byte);

