dnl $Id$
dnl config.m4 for extension DeepTrace

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(DeepTrace, for DeepTrace support,
dnl Make sure that the comment is aligned:
dnl [  --with-DeepTrace             Include DeepTrace support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(DeepTrace, whether to enable DeepTrace support,
dnl Make sure that the comment is aligned:
[  --enable-DeepTrace           Enable DeepTrace support])

if test "$PHP_DEEPTRACE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-DeepTrace -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/DeepTrace.h"  # you most likely want to change this
  dnl if test -r $PHP_DEEPTRACE/$SEARCH_FOR; then # path given as parameter
  dnl   DEEPTRACE_DIR=$PHP_DEEPTRACE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for DeepTrace files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       DEEPTRACE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$DEEPTRACE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the DeepTrace distribution])
  dnl fi

  dnl # --with-DeepTrace -> add include path
  dnl PHP_ADD_INCLUDE($DEEPTRACE_DIR/include)

  dnl # --with-DeepTrace -> check for lib and symbol presence
  dnl LIBNAME=DeepTrace # you may want to change this
  dnl LIBSYMBOL=DeepTrace # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $DEEPTRACE_DIR/lib, DEEPTRACE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_DEEPTRACELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong DeepTrace lib version or lib not found])
  dnl ],[
  dnl   -L$DEEPTRACE_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(DEEPTRACE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(DeepTrace, DeepTrace.c, $ext_shared)
fi
