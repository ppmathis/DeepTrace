/*
	+-----------------------------------------------------------------------+
	| DeepTrace ( Homepage: https://www.snapserv.net/ )					 	|
	+-----------------------------------------------------------------------+
	| Copyright (c) 2012 P. Mathis (pmathis@snapserv.net)                   |
	+-----------------------------------------------------------------------+
	| License info (CC BY-NC-SA 3.0)										|
	|																		|
	| This code is licensed via a Creative Commons Licence:					|
	| http://creativecommons.org/licenses/by-nc-sa/3.0/						|
	| Means:	- You may alter the code, but have to give the changes back |
	|			- You may not use this work for commercial purposes			|
	|			- You must attribute the work in the manner specified by	|
	|				the author or licensor.									|
	+-----------------------------------------------------------------------+
	| If you like to use this code commercially,							|
	| please contact pmathis@snapserv.net									|
	+-----------------------------------------------------------------------+
*/

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "SAPI.h"
#include "Zend/zend_extensions.h"
#include "Zend/zend_exceptions.h"

#ifndef PHP_DEEPTRACE_H
#define PHP_DEEPTRACE_H

// Export module entry
extern zend_module_entry DeepTrace_module_entry;
#define phpext_DeepTrace_ptr &DeepTrace_module_entry

// Export functions
#ifdef PHP_WIN32
	#define PHP_DEEPTRACE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
	#define PHP_DEEPTRACE_API __attribute__ ((visibility("default")))
#else
	#define PHP_DEEPTRACE_API
#endif

// Zend extension API
#ifndef ZEND_EXT_API
	#define ZEND_EXT_API ZEND_DLEXPORT
#endif

// Compile DeepTrace
#ifdef COMPILE_DL_DEEPTRACE
	ZEND_GET_MODULE(DeepTrace)
#endif

// Thread safety and DeepTrace globals
#ifdef ZTS
	#include "TSRM.h"
	#define DEEPTRACE_G(v) TSRMG(DeepTrace_globals_id, zend_DeepTrace_globals *, v)
	#define DEEPTRACE_TSRMLS_C TSRMLS_C
#else
	#define DEEPTRACE_G(v) (DeepTrace_globals.v)
	#define DEEPTRACE_TSRMLS_C NULL
#endif

// Redefine some zend macros
#undef EX
#define EX(element) execute_data->element
#define EX_T(offset) (*(temp_variable *)((char *) EX(Ts) + offset))

// Module entry and exit points
PHP_MINIT_FUNCTION(DeepTrace);
PHP_MSHUTDOWN_FUNCTION(DeepTrace);
PHP_RINIT_FUNCTION(DeepTrace);
PHP_RSHUTDOWN_FUNCTION(DeepTrace);
PHP_MINFO_FUNCTION(DeepTrace);

// PHP versions
#if PHP_API_VERSION > 20090626
	#define DT_PHP_VERSION 54
#else
	#define DT_PHP_VERSION 53
#endif

// True globals - No thread safety required
static user_opcode_handler_t oldExitHandler = NULL;

// Opcode handler structure
typedef struct {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} dt_opcode_handler_t;

// DeepTrace module globals
ZEND_BEGIN_MODULE_GLOBALS(DeepTrace)
	// Thread name
	char *argv0;

	// Misc variables
	zend_bool infoMode;

	// Exit handler
	dt_opcode_handler_t exitHandler;
	zend_bool throwException;
	zend_class_entry *exitException;
	char *exitExceptionType;
	zend_class_entry exitExceptionClass;

	// Function hashtables
	HashTable *replaced_internal_functions;
	HashTable *misplaced_internal_functions;
ZEND_END_MODULE_GLOBALS(DeepTrace)
extern ZEND_DECLARE_MODULE_GLOBALS(DeepTrace);

// DeepTrace constants
#define DEEPTRACE_VERSION "1.3.1"
#define DEEPTRACE_PROCTITLE_MAX_LEN 128
#define DEEPTRACE_EXIT_EXCEPTION_TYPE "DeepTraceExitException"
#define DEEPTRACE_FUNCTION_REMOVE 1
#define DEEPTRACE_FUNCTION_RENAME 2
#define DEEPTRACE_REMOVE_CLASS 1
#define DEEPTRACE_REMOVE_INTERFACE 2
#define DEEPTRACE_REMOVE_TRAIT 3

// DeepTrace PHP functions
PHP_FUNCTION(dt_set_exit_handler);
PHP_FUNCTION(dt_throw_exit_exception);
PHP_FUNCTION(dt_remove_constant);
PHP_FUNCTION(dt_set_proctitle);
PHP_FUNCTION(dt_show_plain_info);
PHP_FUNCTION(dt_remove_include);
PHP_FUNCTION(dt_rename_function);
PHP_FUNCTION(dt_remove_function);
PHP_FUNCTION(dt_remove_class);
PHP_FUNCTION(dt_remove_interface);
PHP_FUNCTION(dt_remove_trait);

// DeepTrace internal functions
int DeepTrace_exit_handler(ZEND_OPCODE_HANDLER_ARGS);
void DeepTrace_free_handler(zend_fcall_info *fci);
int DeepTrace_destroy_misplaced_functions(zend_hash_key *hash_key TSRMLS_DC);
int DeepTrace_restore_internal_functions(zend_internal_function *func TSRMLS_DC, int numArgs, va_list args, zend_hash_key *hash_key);
int DeepTrace_delete_user_functions(void *dest TSRMLS_DC);

#endif