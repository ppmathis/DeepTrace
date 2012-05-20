dnl $Id$
dnl config.m4 for DeepTrace extension

PHP_ARG_ENABLE(DeepTrace, whether to enable DeepTrace support,
[  --enable-DeepTrace           Enable DeepTrace support])

if test "$PHP_DEEPTRACE" != "no"; then
	PHP_NEW_EXTENSION(DeepTrace, DeepTrace.c DeepTrace_handler.c DeepTrace_misc.c DeepTrace_constants.c DeepTrace_functions.c, $ext_shared)
fi