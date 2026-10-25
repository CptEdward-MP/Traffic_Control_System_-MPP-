#ifndef LOG_H_
#define LOG_H_

typedef struct traffic_t traffic_t;
typedef enum traffic_lane_select traffic_lane_select;

void input_init(void);
void input_cleanup(void);
int  input_get_key(void);

void traffic_print_state(traffic_t* i);

traffic_lane_select car_jumper_sensor(void);


#endif
