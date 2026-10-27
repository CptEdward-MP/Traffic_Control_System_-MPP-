#include "traffic_manager.h"


#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

/* #define USB_DEV "/dev/ttyACM0" */

/* typedef struct { */
/*     int fd; */
/*     traffic_t state; */
/* } traffic_usb_t; */


/* void traffic_usb_init(traffic_usb_t *i) */
/* { */
/*     i->fd = open(USB_DEV, O_RDONLY | O_NOCTTY); */

/*     struct termios tty; */
/*     tcgetattr(i->fd, &tty); */
/*     cfmakeraw(&tty); */
/*     tcsetattr(i->fd, TCSANOW, &tty); */
/* } */


/* void traffic_usb_read(traffic_usb_t *i) */
/* { */
/*     // pointer to the data */
/*     // (interpreted as a u8* pointer for byte level granularity) */
/*     u8 *data = (u8 *)&i->state; */
/*     size_t received = 0; */

/*     while (received < sizeof(i->state)) { */
/*         ssize_t n = read( */
/*             i->fd, */
/*             data + received, */
/*             sizeof(i->state) - received */
/*         ); */

/*         if (n > 0) */
/*             received += n; */
/*     } */
/* } */



static struct termios old_terminal;
static int old_flags;

void input_init(void)
{
    struct termios new_terminal;

    tcgetattr(STDIN_FILENO, &old_terminal);
    new_terminal = old_terminal;

    new_terminal.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal);

    old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);
}

void input_cleanup(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal);
    fcntl(STDIN_FILENO, F_SETFL, old_flags);
}

int input_get_key(void)
{
    char c;

    if (read(STDIN_FILENO, &c, 1) == 1)
        return c;

    return -1;
}


int pal_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int result = vprintf(format, args);

    va_end(args);

    return result;
}


void traffic_print_state(traffic_t *i)
{
    /* pal_printf("\033[H\033[2K"); */

    pal_printf(
        "CAR COUNT: %d %d %d | GREEN: %d | YELLOW: %d | LANE: %d | STATE: %d\n",
        i->traffic_car_body[0],
        i->traffic_car_body[1],
        i->traffic_car_body[2],
        i->greenlight_countdown,
        i->yellowlight_countdown,
        i->lane_select + 1,
        i->traffic_state
    );

    /* fflush(stdout); */
}

static f_inline
u8 lane0_sensor(void)
{
  /**
   * If i->lane_select == sensor_data
   * return 0 */
   /* else */
     return 4;
}

static f_inline
u8 lane1_sensor(void)
{
  /**
   * If i->lane_select == sensor_data
   * return 1 */
   /* else */
     return 4;
}


static f_inline
u8 lane2_sensor(void)
{
  /**
   * If i->lane_select == sensor_data
   * return 2 */
   /* else */
     return 4;

}

static f_inline
u8 lane3_sensor(void)
{
  /**
   * If i->lane_select == sensor_data
   * return 2 */
   /* else */
     return 4;

}




u8 car_jumper_sensor(traffic_t* i)
{

    // 5 means undefined
    u8 traffic_lane0_sensor = 5;
    u8 traffic_lane1_sensor = 5;
    u8 traffic_lane2_sensor = 5;
    u8 traffic_lane3_sensor = 5;

    traffic_lane0_sensor = lane0_sensor();
    traffic_lane1_sensor = lane1_sensor();
    traffic_lane2_sensor = lane2_sensor();
    traffic_lane3_sensor = lane3_sensor();

    if (traffic_lane0_sensor < 4)
      return 0;

    if (traffic_lane1_sensor < 4)
      return 1;

    if (traffic_lane2_sensor < 4)
      return 2;

    if (traffic_lane3_sensor < 4)
      return 3;



    return 4;

    /* for (u8 count = 0; count < 3; count++) */
    /* { */
    /*     if (i->lane_select == 0 && traffic_lane0_sensor == 4) continue; */
    /*     if (i->lane_select == 1 && traffic_lane1_sensor == 4) continue; */
    /*     if (i->lane_select == 2 && traffic_lane2_sensor == 4) continue; */

    /*     if (traffic_lane0_sensor == 4) */
    /*     { */
    /*         /\* i->lane_jumpers_s.jumped_cars[ *\/ */
    /*         /\*     i->lane_jumpers_s.car_count].reached_lane = LANE0; *\/ */

    /*         return 0; */
    /*     } */

    /*     if (traffic_lane1_sensor == 4) */
    /*     { */
    /*         /\* i->lane_jumpers_s.jumped_cars[ *\/ */
    /*         /\*     i->lane_jumpers_s.car_count].reached_lane = LANE1; *\/ */

    /*         return 1; */
    /*     } */

    /*     if (traffic_lane2_sensor == 4) */
    /*     { */
    /*         /\* i->lane_jumpers_s.jumped_cars[ *\/ */
    /*         /\*     i->lane_jumpers_s.car_count].reached_lane = LANE2; *\/ */

    /*         return 2; */
    /*     } */
    /* } */
}
