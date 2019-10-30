void display_init(void);
void display_goto_line(int line);
void display_send_string(const char *string);

void display_transport_reset(void);
void display_transport_set_control(void);
void display_transport_set_data(void);
void display_transport_write(char byte);

