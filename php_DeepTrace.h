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
#include "zend_constants.h"
#include "zend_exceptions.h"
#include "zend_extensions.h"
#include "SAPI.h"

/* Export module entry point */
extern zend_module_entry DeepTrace_module_entry;
#define phpext_DeepTrace_ptr &DeepTrace_module_entry

/* Macro for exporting DeepTrace API functions to other extensions  */
#if defined(__GNUC__) && __GNUC__ >= 4
#define PHP_DEEPTRACE_API __attribute__ ((visibility("default")))
#else
#define PHP_DEEPTRACE_API
#endif

/* Thread-safety for module globals */
#ifdef ZTS
#include "TSRM.h"
#define DEEPTRACE_G(v) TSRMG(DeepTrace_globals_id, zend_DeepTrace_globals *, v)
#else
#define DEEPTRACE_G(v) (DeepTrace_globals.v)
#endif

/* Adjust some zend macros */
#undef EX
#define EX(element) execute_data->element
#define EX_T(offset) (*(temp_variable *)((char *) EX(Ts) + offset))

/* Declare module entry and exit points */
PHP_MINIT_FUNCTION(DeepTrace);
PHP_MSHUTDOWN_FUNCTION(DeepTrace);
PHP_RINIT_FUNCTION(DeepTrace);
PHP_RSHUTDOWN_FUNCTION(DeepTrace);
PHP_MINFO_FUNCTION(DeepTrace);

/*
 * Detect current PHP version
 * Not yet used, added for backwards compatibility
 */
#if PHP_API_VERSION > 20090626
#define DEEPTRACE_PHP_VERSION 54
#else
#define DEEPTRACE_PHP_VERSION 53
#error "DeepTrace 2 does not support PHP 5.3 anymore."
#endif

/* DeepTrace opcode handler structure */
typedef struct {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} deeptrace_opcode_handler_t;

/* DeepTrace module globals */
ZEND_BEGIN_MODULE_GLOBALS(DeepTrace)
	char* argv0;

	long exitMode;
	deeptrace_opcode_handler_t exitHandler;
	user_opcode_handler_t exitOldHandler;
	zend_class_entry *exitException;

	HashTable *replaced_internal_functions;
	HashTable *misplaced_internal_functions;
	HashTable *constantCache;

	zend_bool fixStaticMethodCalls;
ZEND_END_MODULE_GLOBALS(DeepTrace)
extern ZEND_DECLARE_MODULE_GLOBALS(DeepTrace)

/* DeepTrace internal constants */
#define DEEPTRACE_VERSION "2.0.0"
#define DEEPTRACE_PROCTITLE_MAX_LEN 256
#define DEEPTRACE_TEMP_FUNCNAME "__dt_temporary_function__"

#define DEEPTRACE_PHPINFO_HTML 0
#define DEEPTRACE_PHPINFO_TEXT 1

#define DEEPTRACE_EXIT_NORMAL		0
#define DEEPTRACE_EXIT_HANDLER		1
#define DEEPTRACE_EXIT_EXCEPTION	2

#define DEEPTRACE_FUNCTION_SET_STATIC_VAR	0
#define DEEPTRACE_FUNCTION_REMOVE			1
#define DEEPTRACE_FUNCTION_RENAME			2

/*
 * Prints debug information about the constant cache
 * if defined. Should not be used in production - tests
 * will fail if activated.
 */
// #define DEEPTRACE_DEBUG_CONSTANT_CACHE

/* DeepTrace internal macros */
#define DEEPTRACE_DECL_STRING_PARAM(p)			char *p; int p##_len;
#define DEEPTRACE_DECL_HANDLER_PARAM(p)			deeptrace_opcode_handler_t p;
#define DEEPTRACE_STRING_PARAM(p)				&p, &p##_len
#define DEEPTRACE_HANDLER_PARAM(p)				&p.fci, &p.fcc

#define DT_ADD_MAGIC_METHOD(ce, method, fe) { \
	if ((strcmp((method), (ce)->name) == 0) || (strcmp((method), "__construct") == 0)) { (ce)->constructor	= (fe); (fe)->common.fn_flags = ZEND_ACC_CTOR; } \
	else if (strcmp((method), "__destruct") == 0) {	(ce)->destructor	= (fe); (fe)->common.fn_flags = ZEND_ACC_DTOR; } \
	else if (strcmp((method), "__clone") == 0)  {	(ce)->clone			= (fe); (fe)->common.fn_flags = ZEND_ACC_CLONE; } \
	else if (strcmp((method), "__get") == 0)		(ce)->__get			= (fe); \
	else if (strcmp((method), "__set") == 0)		(ce)->__set			= (fe); \
	else if (strcmp((method), "__call") == 0)		(ce)->__call		= (fe); \
}

#define DT_DEL_MAGIC_METHOD(ce, fe) { \
	if ((ce)->constructor == (fe))			(ce)->constructor	= NULL; \
	else if ((ce)->destructor == (fe))		(ce)->destructor	= NULL; \
	else if ((ce)->clone == (fe))			(ce)->clone			= NULL; \
	else if ((ce)->__get == (fe))			(ce)->__get			= NULL; \
	else if ((ce)->__set == (fe))			(ce)->__set			= NULL; \
	else if ((ce)->__call == (fe))			(ce)->__call		= NULL; \
}

/* DeepTrace internal functions */
int DeepTrace_exit_handler(ZEND_OPCODE_HANDLER_ARGS);
int DeepTrace_constant_handler(ZEND_OPCODE_HANDLER_ARGS);
int DeepTrace_static_method_call_handler(ZEND_OPCODE_HANDLER_ARGS);
void DeepTrace_exit_cleanup();
void DeepTrace_functions_cleanup();
void DeepTrace_constants_cleanup();

/* DeepTrace PHP functions */
PHP_FUNCTION(dt_phpinfo_mode);
PHP_FUNCTION(dt_set_proctitle);
PHP_FUNCTION(dt_exit_mode);
PHP_FUNCTION(dt_exit_fetch_exception);
PHP_FUNCTION(dt_inspect_zval);
PHP_FUNCTION(dt_remove_include);
PHP_FUNCTION(dt_remove_function);
PHP_FUNCTION(dt_rename_function);
PHP_FUNCTION(dt_set_static_function_variable);
PHP_FUNCTION(dt_remove_class);
PHP_FUNCTION(dt_destroy_class_data);
PHP_FUNCTION(dt_clear_constant_cache);
PHP_FUNCTION(dt_remove_constant);
PHP_FUNCTION(dt_get_constant_cache_stats);
PHP_FUNCTION(dt_destroy_class_consts);
PHP_FUNCTION(dt_add_method);
PHP_FUNCTION(dt_rename_method);
PHP_FUNCTION(dt_remove_method);
PHP_FUNCTION(dt_set_static_method_variable);
PHP_FUNCTION(dt_fix_static_method_calls);

#endif
