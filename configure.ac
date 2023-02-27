AC_INIT([timewave], [1.3], [kennedy.greg@gmail.com])

AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC

AC_CHECK_LIB([m],[atan2])

dnl Check for SDL
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS -lSDL_mixer -lSDL_image"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
