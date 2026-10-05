#include "traffic_manager.h"

#include <fcntl.h>
/* #include <stdio.h> */
/* #include <termios.h> */
/* #include <unistd.h> */

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
    pal_printf("\033[2J\033[H");

    pal_printf(
        "CAR COUNT: %d %d %d | GREEN: %d | YELLOW: %d | LANE: %d | STATE: %d\r",
        i->traffic_car_body[0],
        i->traffic_car_body[1],
        i->traffic_car_body[2],
        i->greenlight_countdown,
        i->yellowlight_countdown,
        i->lane_select + 1,
        i->traffic_state
    );

    fflush(stdout);
}
