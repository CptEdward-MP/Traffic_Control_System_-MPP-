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
