/*
 * +------------------------------------------------------------------------+
 * | DeepTrace (Website: http://www.snapserv.net/)							|
 * +------------------------------------------------------------------------+
 * | Copyright (c) 2012-2013 	P. Mathis (pmathis@snapserv.net)			|
 * |							Y. Khalil (dev@pp3345.net)					|
 * +------------------------------------------------------------------------+
 * | Licensed under the Apache License, Version 2.0 (the "License");		|
 * | you may not use this file except in compliance with the License.		|
 * | You may obtain a copy of the License at								|
 * |																		|
 * |	http://www.apache.org/licenses/LICENSE-2.0							|
 * |																		|
 * | Unless required by applicable law or agreed to in writing, software	|
 * | distributed under the License is distributed on an "AS IS" BASIS,		|
 * | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or		|
 * | implied. See the License for the specific language governing 			|
 * | permissions and limitations under the License.							|
 * +------------------------------------------------------------------------+
 */

#ifndef PHP_DEEPTRACE_H
#define PHP_DEEPTRACE_H

/* Includes */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_extensions.h"
#include "SAPI.h"

/* Export module entry point */
extern zend_module_entry DeepTrace_module_entry;
#define phpext_DeepTrace_ptr &DeepTrace_module_entry

/* Macro for exporting DeepTrace API functions to other extensions  */
#ifdef PHP_WIN32
#define PHP_DEEPTRACE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_DEEPTRACE_API __attribute__ ((visibility("default")))
#endif

/* Thread-safety for module globals */
#ifdef ZTS
#include "TSRM.h"
#define DEEPTRACE_G(v) TSRMG(DeepTrace_globals_id, zend_DeepTrace_globals *, v)
#else
#define DEEPTRACE_G(v) (DeepTrace_globals.v)
#endif

/* Declare module entry and exit points */
PHP_MINIT_FUNCTION(DeepTrace);
PHP_MSHUTDOWN_FUNCTION(DeepTrace);
PHP_RINIT_FUNCTION(DeepTrace);
PHP_RSHUTDOWN_FUNCTION(DeepTrace);
PHP_MINFO_FUNCTION(DeepTrace);

/*
 * Detect current PHP version
 * Not yet used, added for backwards compatiblity
 */
#if PHP_API_VERSION > 20090626
#define DT_PHP_VERSION 54
#else
#define DT_PHP_VERSION 53
#error "DeepTrace does not support PHP 5.3 anymore."
#endif

/* DeepTrace module globals */
ZEND_BEGIN_MODULE_GLOBALS(DeepTrace)
	char* argv0;
ZEND_END_MODULE_GLOBALS(DeepTrace)
extern ZEND_DECLARE_MODULE_GLOBALS(DeepTrace)

/* DeepTrace internal constants */
#define DEEPTRACE_VERSION "2.0.0"
#define DEEPTRACE_PHPINFO_HTML 0
#define DEEPTRACE_PHPINFO_TEXT 1
#define DEEPTRACE_PROCTITLE_MAX_LEN 256

/* DeepTrace internal macros */
#define DEEPTRACE_DECL_STRING_PARAM(p)			char *p; int p##_len;
#define DEEPTRACE_STRING_PARAM(p)				&p, &p##_len

/* DeepTrace PHP functions */
PHP_FUNCTION(dt_phpinfo_mode);
PHP_FUNCTION(dt_set_proctitle);

#endif
