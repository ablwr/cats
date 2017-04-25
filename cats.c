
#if defined (_WIN32) && !defined(WIN32)
#    define WIN32
#else
     /* fileno() */
#    define _POSIX_C_SOURCE 1
     /* usleep() */
#    define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#    include <process.h>
#    include <io.h>

    /* usleep() doesn't exist on MSVC, instead use Sleep() from Win32 API */
#    define usleep(a) Sleep((a) / 1000)

#    define execv(a, b) do { i = _spawnv(_P_WAIT, (a), (b)); if (i != -1) return i; } while(0)
#    define execvp(a, b) do { i = _spawnvp(_P_WAIT, (a), (b)); if (i != -1) return i; } while(0)

#else
#    include <unistd.h>
#endif

#ifndef WIN32
#    include <sys/ioctl.h>
#else
#    include <windows.h>
HANDLE WIN_CONSOLE;
#endif

/* SunOS defines winsize in termios.h */
#if defined(__sun) && defined(__SVR4)
#    include <sys/termios.h>
#endif

#ifndef CAT_SPEED
#    define CAT_SPEED 100
#endif

int term_width(void);
void init_space(void);
void open_term();
void move_to_top(void);
void line_at(int start_x, const char *s);
void clear_cat(int x);

typedef void (*draw_fn_t) (int x);
void draw_std(int x);
draw_fn_t select_command(int argc, char **argv);

FILE *TERM_FH;
int TERM_WIDTH;
unsigned int FRAME_TIME;

int main(int argc, char **argv)
{
    int i;
    char *tmp;
    unsigned int cat_speed;
    draw_fn_t draw_fn;

    tmp = getenv("CAT_SPEED");
    if (!tmp || sscanf(tmp, "%u", &cat_speed) != 1) {
        cat_speed = CAT_SPEED;
    }
    open_term();
    TERM_WIDTH = term_width();
    FRAME_TIME = 1000 * 1000 * 10 / (cat_speed + TERM_WIDTH + 1);

    draw_fn = select_command(argc, argv);
    init_space();
    for (i = -20; i < TERM_WIDTH; i++) {
        draw_fn(i);
        clear_cat(i);
    }
    move_to_top();
    fflush(TERM_FH);
}

draw_fn_t select_command(int argc, char **argv)
{
  int i;

  for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-')
          continue;
      break;
  }
  return draw_std;
}

void init_space(void)
{
  fputs("\n\n\n\n\n\n\n\n\n\n\n", TERM_FH);   /* 8 lines, to not remove the PS1 line */
  fflush(TERM_FH);
}

void open_term()
{
#ifndef WIN32
    TERM_FH = fopen("/dev/tty", "w");
    if (!TERM_FH)
        TERM_FH = stdout;
#else
    TERM_FH = fopen("CONOUT$", "w+");
    WIN_CONSOLE = (HANDLE)_get_osfhandle(fileno(TERM_FH));
#endif
}

int term_width(void)
{
#ifndef WIN32
    struct winsize w;
    ioctl(fileno(TERM_FH), TIOCGWINSZ, &w);
    return w.ws_col;
#else
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(WIN_CONSOLE, &ci);
    return ci.dwSize.X;
#endif
}

void move_to_top(void)
{
#ifndef WIN32
    fprintf(TERM_FH, "\033[11A");
#else
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(WIN_CONSOLE, &ci);
    ci.dwCursorPosition.X = 0;
    ci.dwCursorPosition.Y -= 11;
    SetConsoleCursorPosition(WIN_CONSOLE, ci.dwCursorPosition);
#endif
}

void move_to_x(int x)
{
#ifndef WIN32
    fprintf(TERM_FH, "\033[%dC", x);
#else
    CONSOLE_SCREEN_BUFFER_INFO ci;
    GetConsoleScreenBufferInfo(WIN_CONSOLE, &ci);
    ci.dwCursorPosition.X = x;
    SetConsoleCursorPosition(WIN_CONSOLE, ci.dwCursorPosition);
#endif
}

void line_at(int start_x, const char *s)
{
    int x;
    size_t i;
    if (start_x > 1)
        move_to_x(start_x);
    for (x = start_x, i = 0; i < strlen(s); x++, i++) {
        if (x > 0 && x < TERM_WIDTH)
            fputc(s[i], TERM_FH);
    }
#ifdef WIN32
    /*
     * It seems Windows wraps on whe cursor when it's about to overflow,
     * rather than after it has overflown (unless the overflowing character
     * is a newline), as other systems seems to do.
     */
    if (x < TERM_WIDTH)
#endif
    fputc('\n', TERM_FH);
    fflush(TERM_FH);
}

void draw_std(int x)
{
    /* *INDENT-OFF* */
    move_to_top();
    line_at(x, "               ________________                 ");
    line_at(x, "         _|::||                |                ");
    line_at(x, "      _|::|   |  I Love Cats!  |                ");
    line_at(x, "     |::|     |________________|                ");
    line_at(x, "                               |        /\\**/\\ ");
    line_at(x, "                               |       _( >_> ) ");
    line_at(x, "                               |  (_//   u--u)  ");
    line_at(x, "                                 \\==(  ___||)   ");
    if (x % 2) {
    line_at(x, "                             ,dP /b/=( /P /b\\  ");
    line_at(x, "                             |8 || 8\\=== || 8  ");
    line_at(x, "                             'b,  ,P  'b,  ,P   ");
    } else {
    line_at(x, "                             ,dP /b/=( /P /b\\  ");
    line_at(x, "                             |8 == 8\\=== == 8  ");
    line_at(x, "                             'b,  ,P  'b,  ,P   ");
    }
    /* *INDENT-ON* */
    usleep(FRAME_TIME);
}

void clear_cat(int x)
{
    move_to_top();
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
    line_at(x, "  ");
}
