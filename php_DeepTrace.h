/*
  +-----------------------------------------------------------------------+
  | DeepTrace v1.1 ( Homepage: https://www.snapserv.net/ )        |
  +-----------------------------------------------------------------------+
  | Copyright (c) 2012 P. Mathis (pmathis@snapserv.net)                   |
  +-----------------------------------------------------------------------+
  | License info (CC BY-NC-SA 3.0)                    |
  |                                   |
  | This code is licensed via a Creative Commons Licence:         |
  | http://creativecommons.org/licenses/by-nc-sa/3.0/           |
  | Means:  - You may alter the code, but have to give the changes back |
  |     - You may not use this work for commercial purposes     |
  |     - You must attribute the work in the manner specified by  |
  |       the author or licensor.                 |
  +-----------------------------------------------------------------------+
  | If you like to use this code commercially,              |
  | please contact pmathis@snapserv.net                 |
  +-----------------------------------------------------------------------+
*/

#ifndef PHP_DEEPTRACE_H
#define PHP_DEEPTRACE_H

extern zend_module_entry DeepTrace_module_entry;
#define phpext_DeepTrace_ptr &DeepTrace_module_entry

#ifdef PHP_WIN32
#	define PHP_DEEPTRACE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_DEEPTRACE_API __attribute__ ((visibility("default")))
#else
#	define PHP_DEEPTRACE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(DeepTrace);
PHP_MSHUTDOWN_FUNCTION(DeepTrace);
PHP_MINFO_FUNCTION(DeepTrace);

#ifdef ZTS
#define DEEPTRACE_G(v) TSRMG(DeepTrace_globals_id, zend_DeepTrace_globals *, v)
#else
#define DEEPTRACE_G(v) (DeepTrace_globals.v)
#endif

#endif	/* PHP_DEEPTRACE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
 