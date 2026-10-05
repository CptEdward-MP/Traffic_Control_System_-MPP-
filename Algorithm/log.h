#ifndef LOG_H_
#define LOG_H_

typedef struct traffic_t traffic_t;


void input_init(void);
void input_cleanup(void);
int  input_get_key(void);

void traffic_print_state(traffic_t* i);

#endif
