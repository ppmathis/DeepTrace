dnl $Id$
dnl config.m4 for DeepTrace extension

PHP_ARG_ENABLE(DeepTrace, whether to enable DeepTrace support,
[	--enable-DeepTrace						Enable DeepTrace support], no, yes)

PHP_ARG_ENABLE(DeepTrace-legacy-proctitle, whether to force the legacy proctitle call,
[	--enable-DeepTrace-legacy-proctitle		Force legacy proctitle call], no, no)

if test "$PHP_DEEPTRACE" != "no"; then
	if test "$PHP_DEEPTRACE_LEGACY_PROCTITLE" == "no"; then
		AC_CACHE_VAL(_cv_dt_system_has_setproctitle, [
		AC_CHECK_FUNCS(setproctitle, [
			_cv_dt_system_has_setproctitle=yes
			break
		],[
			_cv_dt_system_has_setproctitle=no
		])])
	
		AC_MSG_CHECKING([if your OS has setproctitle])
		if test "$_cv_dt_system_has_setproctitle" = "yes"; then
			AC_MSG_RESULT(yes)
			AC_DEFINE(DEEPTRACE_SYSTEM_PROVIDES_SETPROCTITLE, 1, [Define if your system has setproctitle])
		else
			AC_MSG_RESULT(no)
		fi
	fi
	
	PHP_NEW_EXTENSION(DeepTrace, DeepTrace.c DeepTrace_misc.c, $ext_shared)
fi