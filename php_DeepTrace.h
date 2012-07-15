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

#ifndef PHP_DEEPTRACE_H
#define PHP_DEEPTRACE_H

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "SAPI.h"
#include "Zend/zend_extensions.h"
#include "Zend/zend_exceptions.h"

extern HashTable *global_auto_globals_table;

/* --- DEEPTRACE CONFIGURATION --- */
/* Identation = depends on the constant one level above */
#define DEEPTRACE_EXIT_MANIPULATION 								/* Toggle support for hooking exit()/die() calls */
#	define DEEPTRACE_EXIT_EXCEPTION_TYPE "DeepTraceExitException"	/* Specify the default exception name when hooking exit()/die() calls */
#define DEEPTRACE_CONSTANT_MANIPULATION 							/* Toggle support for constant manipulation */
#	define DEEPTRACE_FIX_RUN_TIME_CACHE 							/* Toggle custom runtime cache. Should be turned on when working with constants */
//#	define DEEPTRACE_DEBUG_CACHE									/* Toggle debug output related to custom runtime cache */
#define DEEPTRACE_FUNCTION_MANIPULATION 							/* Toggle support for function manipulation */
#define DEEPTRACE_CLASS_MANIPULATION 								/* Toggle support for class manipulation */
#define DEEPTRACE_INCLUDE_MANIPULATION								/* Toggle support for include manipulation */
#define DEEPTRACE_INFO_MANIPULATION 								/* Toggle support for info manipulation */
#define DEEPTRACE_THREAD_SUPPORT 									/* Toggle support for threads */
//#define DEEPTRACE_CUSTOM_SUPERGLOBALS	 							/* Toggle support for custom superglobals */
#define DEEPTRACE_METHOD_MANIPULATION								/* Toggle support for method manipulation */
#if ZEND_DEBUG
#	define DEEPTRACE_DEBUG_MEMORY									/* Toggle support for memory debugging */
#endif
/* --- END OF DEEPTRACE CONFIGURATION --- */

/* Export module entry point */
extern zend_module_entry DeepTrace_module_entry;
#define phpext_DeepTrace_ptr &DeepTrace_module_entry

/* Export functions */
#ifdef PHP_WIN32
#	define PHP_DEEPTRACE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_DEEPTRACE_API __attribute__ ((visibility("default")))
#else
#	define PHP_DEEPTRACE_API
#endif

/* Zend extension API */
#ifndef ZEND_EXT_API
#	define ZEND_EXT_API ZEND_DLEXPORT
#endif

/* Compile DeepTrace */
/*#ifdef COMPILE_DL_DEEPTRACE
	ZEND_GET_MODULE(DeepTrace)
#endif*/

/* Thread safety */
#ifdef ZTS
#	include "TSRM.h"
#	define DEEPTRACE_G(v) TSRMG(DeepTrace_globals_id, zend_DeepTrace_globals *, v)
#	define DEEPTRACE_TSRMLS_C TSRMLS_C
#else
#	define DEEPTRACE_G(v) (DeepTrace_globals.v)
#	define DEEPTRACE_TSRMLS_C NULL
#endif

/* Redefine some zend macros */
#undef EX
#define EX(element) execute_data->element
#define EX_T(offset) (*(temp_variable *)((char *) EX(Ts) + offset))

/* DeepTrace entry and exit points */
PHP_MINIT_FUNCTION(DeepTrace);
PHP_MSHUTDOWN_FUNCTION(DeepTrace);
PHP_RINIT_FUNCTION(DeepTrace);
PHP_RSHUTDOWN_FUNCTION(DeepTrace);
PHP_MINFO_FUNCTION(DeepTrace);

/* Detect PHP version */
#if PHP_API_VERSION > 20090626
#	define DT_PHP_VERSION 54
#else
#	define DT_PHP_VERSION 53
#endif

/* True globals (no threadsafety) */
#ifdef DEEPTRACE_EXIT_MANIPULATION
	static user_opcode_handler_t oldExitHandler = NULL;
#endif

/* Opcode handler structure */
typedef struct {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} dt_opcode_handler_t;

/* DeepTrace globals */
ZEND_BEGIN_MODULE_GLOBALS(DeepTrace)
	/* Info manipulation */
#	ifdef DEEPTRACE_INFO_MANIPULATION
	zend_bool infoMode;
#	endif

	/* Thread support */
# 	ifdef DEEPTRACE_THREAD_SUPPORT
		char* argv0;
#	endif

#	ifdef DEEPTRACE_EXIT_MANIPULATION
	/* Exit handler */
		dt_opcode_handler_t exitHandler;
		zend_bool throwException;
		zend_class_entry *exitException;
		char *exitExceptionType;
		zend_class_entry exitExceptionClass;
#	endif

	/* Function hashtables */
# 	ifdef DEEPTRACE_FUNCTION_MANIPULATION
		HashTable *replaced_internal_functions;
		HashTable *misplaced_internal_functions;
#	endif

	/* Custom runtime cache */
# 	ifdef DEEPTRACE_CONSTANT_MANIPULATION
# 		ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
			HashTable *constantCache;
		#endif
# 	endif
ZEND_END_MODULE_GLOBALS(DeepTrace)
extern ZEND_DECLARE_MODULE_GLOBALS(DeepTrace);

/* DeepTrace internal constants */
#define DEEPTRACE_VERSION "1.4.1"

#ifdef DEEPTRACE_THREAD_SUPPORT
#	define DEEPTRACE_PROCTITLE_MAX_LEN 256
#endif

#ifdef DEEPTRACE_FUNCTION_MANIPULATION
#	define DEEPTRACE_FUNCTION_REMOVE 1
#	define DEEPTRACE_FUNCTION_RENAME 2
#endif

#ifdef DEEPTRACE_CLASS_MANIPULATION
#	define DEEPTRACE_REMOVE_CLASS 1
#	define DEEPTRACE_REMOVE_INTERFACE 2
#	define DEEPTRACE_REMOVE_TRAIT 3
#endif

/* DeepTrace PHP functions */
#ifdef DEEPTRACE_EXIT_MANIPULATION
	PHP_FUNCTION(dt_set_exit_handler);
	PHP_FUNCTION(dt_throw_exit_exception);
#endif
#ifdef DEEPTRACE_CONSTANT_MANIPULATION
	PHP_FUNCTION(dt_remove_constant);
#	ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
		PHP_FUNCTION(dt_get_cache_size);
		PHP_FUNCTION(dt_clear_cache);
#	endif
#endif
#ifdef DEEPTRACE_THREAD_SUPPORT
	PHP_FUNCTION(dt_set_proctitle);
#endif
#ifdef DEEPTRACE_INFO_MANIPULATION
	PHP_FUNCTION(dt_show_plain_info);
#endif
#ifdef DEEPTRACE_INCLUDE_MANIPULATION
	PHP_FUNCTION(dt_remove_include);
#endif
#ifdef DEEPTRACE_FUNCTION_MANIPULATION
	PHP_FUNCTION(dt_rename_function);
	PHP_FUNCTION(dt_remove_function);
	PHP_FUNCTION(dt_destroy_function_data);
#endif
#ifdef DEEPTRACE_CLASS_MANIPULATION
	PHP_FUNCTION(dt_remove_class);
	PHP_FUNCTION(dt_destroy_class_data);
#endif
#ifdef DEEPTRACE_CUSTOM_SUPERGLOBALS
	PHP_FUNCTION(dt_get_superglobals);
#endif
#ifdef DEEPTRACE_METHOD_MANIPULATION
	PHP_FUNCTION(dt_add_method);
	PHP_FUNCTION(dt_rename_method);
	PHP_FUNCTION(dt_remove_method);
#endif
#ifdef DEEPTRACE_DEBUG_MEMORY
	PHP_FUNCTION(zend_mem_check);
#endif

/* DeepTrace internal functions */
#ifdef DEEPTRACE_EXIT_MANIPULATION
	int DeepTrace_exit_handler(ZEND_OPCODE_HANDLER_ARGS);
	void DeepTrace_free_handler(zend_fcall_info *fci);
#endif
#ifdef DEEPTRACE_FUNCTION_MANIPULATION
	int DeepTrace_destroy_misplaced_functions(zend_hash_key *hash_key TSRMLS_DC);
	int DeepTrace_restore_internal_functions(zend_internal_function *func TSRMLS_DC, int numArgs, va_list args, zend_hash_key *hash_key);
	int DeepTrace_delete_user_functions(void *dest TSRMLS_DC);
#endif
#ifdef DEEPTRACE_CONSTANT_MANIPULATION
#	if DT_PHP_VERSION == 54
#		ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
			int DeepTrace_constant_handler(ZEND_OPCODE_HANDLER_ARGS);
			int DeepTrace_destroy_cache_entries(zend_hash_key *hash_key TSRMLS_DC);
#		endif
#	endif
#endif
#ifdef DEEPTRACE_CUSTOM_SUPERGLOBALS
	int dt_register_superglobal(char* variableName, int len);
#endif
#ifdef DEEPTRACE_METHOD_MANIPULATION
#	define DT_TEMP_FUNCNAME "__dt_temporary_function__"

	zend_function* dt_get_method_prototype(zend_class_entry *ce, char* func, int func_len TSRMLS_DC);
	int dt_generate_lambda_method(char *arguments, int argumentsLen, char *phpcode, int phpcodeLen, zend_function **pfe TSRMLS_DC);
	int dt_fetch_class(char* className, int classLen, zend_class_entry **pce TSRMLS_DC);
	int dt_fetch_class_int(char* className, int classLen, zend_class_entry **pce TSRMLS_DC);
	int dt_fetch_class_method(char* className, int classLen, char* methodName, int methodLen, zend_class_entry **pce, zend_function **pfe TSRMLS_DC);
	int dt_update_children_methods(zend_class_entry *ce TSRMLS_DC, int numArgs, va_list args, zend_hash_key* hashKey);
	int dt_clean_children_methods(zend_class_entry *ce TSRMLS_DC, int numArgs, va_list args, zend_hash_key *hashKey);

#	define DT_ADD_MAGIC_METHOD(ce, method, fe) { \
		if ((strcmp((method), (ce)->name) == 0) || (strcmp((method), "__construct") == 0)) { (ce)->constructor	= (fe); (fe)->common.fn_flags = ZEND_ACC_CTOR; } \
		else if (strcmp((method), "__destruct") == 0) {	(ce)->destructor	= (fe); (fe)->common.fn_flags = ZEND_ACC_DTOR; } \
		else if (strcmp((method), "__clone") == 0)  {	(ce)->clone			= (fe); (fe)->common.fn_flags = ZEND_ACC_CLONE; } \
		else if (strcmp((method), "__get") == 0)		(ce)->__get			= (fe); \
		else if (strcmp((method), "__set") == 0)		(ce)->__set			= (fe); \
		else if (strcmp((method), "__call") == 0)		(ce)->__call		= (fe); \
	}

#	define DT_DEL_MAGIC_METHOD(ce, fe) { \
		if ((ce)->constructor == (fe))			(ce)->constructor	= NULL; \
		else if ((ce)->destructor == (fe))		(ce)->destructor	= NULL; \
		else if ((ce)->clone == (fe))			(ce)->clone			= NULL; \
		else if ((ce)->__get == (fe))			(ce)->__get			= NULL; \
		else if ((ce)->__set == (fe))			(ce)->__set			= NULL; \
		else if ((ce)->__call == (fe))			(ce)->__call		= NULL; \
	}
#endif
#endif
