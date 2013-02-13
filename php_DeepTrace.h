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
#include "zend_closures.h"
#include "zend_constants.h"
#include "zend_exceptions.h"
#include "zend_extensions.h"
#include "SAPI.h"

/* Export module entry point */
extern zend_module_entry DeepTrace_module_entry;
#define phpext_DeepTrace_ptr &DeepTrace_module_entry

/*
 * Detect current PHP version
 * Not yet used, added for backwards compatibility
 */
#if PHP_API_VERSION >= 20121113
#define DEEPTRACE_PHP_VERSION 55
#elif PHP_API_VERSION >= 20090626
#define DEEPTRACE_PHP_VERSION 54
#else
#define DEEPTRACE_PHP_VERSION 53
#error "DeepTrace 2 does not support PHP 5.3 anymore."
#endif

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
#if DEEPTRACE_PHP_VERSION >= 55
#define EX_T(offset) (*EX_TMP_VAR(execute_data, offset))
#else
#define EX_T(offset) (*(temp_variable *)((char *) EX(Ts) + offset))
#endif

/* Zend closure structure */
typedef struct _zend_closure {
	zend_object std;
	zend_function func;
	HashTable *debug_info;
} zend_closure;

/* Declare module entry and exit points */
PHP_MINIT_FUNCTION(DeepTrace);
PHP_MSHUTDOWN_FUNCTION(DeepTrace);
PHP_RINIT_FUNCTION(DeepTrace);
PHP_RSHUTDOWN_FUNCTION(DeepTrace);
PHP_MINFO_FUNCTION(DeepTrace);

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
	HashTable *misplaced_internal_methods;
ZEND_END_MODULE_GLOBALS(DeepTrace)
extern ZEND_DECLARE_MODULE_GLOBALS(DeepTrace)

/* DeepTrace internal constants */
#define DEEPTRACE_VERSION "2.1.0"
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

#define DEEPTRACE_MISPLACED_METHOD_ADD		0
#define DEEPTRACE_MISPLACED_METHOD_REMOVE	1

/* DeepTrace internal macros */
#define DEEPTRACE_DECL_STRING_PARAM(p)			char *p; int p##_len;
#define DEEPTRACE_DECL_HANDLER_PARAM(p)			deeptrace_opcode_handler_t p;
#define DEEPTRACE_STRING_PARAM(p)				&p, &p##_len
#define DEEPTRACE_HANDLER_PARAM(p)				&p.fci, &p.fcc

#define DEEPTRACE_ADD_MAGIC_METHOD(ce, mname, mname_len, fe, orig_fe) { \
	if (!strncmp((mname), ZEND_CLONE_FUNC_NAME, (mname_len))) { \
		(ce)->clone = (fe); (fe)->common.fn_flags |= ZEND_ACC_CLONE; \
	} else if (!strncmp((mname), ZEND_CONSTRUCTOR_FUNC_NAME, (mname_len))) { \
		if (!(ce)->constructor || (ce)->constructor == (orig_fe)) { \
			(ce)->constructor = (fe); (fe)->common.fn_flags |= ZEND_ACC_CTOR; \
		} \
	} else if (!strncmp((mname), ZEND_DESTRUCTOR_FUNC_NAME, (mname_len))) { \
		(ce)->destructor = (fe); (fe)->common.fn_flags |= ZEND_ACC_DTOR; \
	} else if (!strncmp((mname), ZEND_GET_FUNC_NAME, (mname_len))) { \
		(ce)->__get = (fe); \
	} else if (!strncmp((mname), ZEND_SET_FUNC_NAME, (mname_len))) { \
		(ce)->__set = (fe); \
	} else if (!strncmp((mname), ZEND_CALL_FUNC_NAME, (mname_len))) { \
		(ce)->__call = (fe); \
	} else if (!strncmp((mname), ZEND_UNSET_FUNC_NAME, (mname_len))) { \
		(ce)->__unset = (fe); \
	} else if (!strncmp((mname), ZEND_ISSET_FUNC_NAME, (mname_len))) { \
		(ce)->__isset = (fe); \
	} else if (!strncmp((mname), ZEND_CALLSTATIC_FUNC_NAME, (mname_len))) { \
		(ce)->__callstatic = (fe); \
	} else if (!strncmp((mname), ZEND_TOSTRING_FUNC_NAME, (mname_len))) { \
		(ce)->__tostring = (fe); \
	} else if ((ce)->name_length == (mname_len)) { \
		char *lowercase_name = emalloc((ce)->name_length + 1); \
		zend_str_tolower_copy(lowercase_name, (ce)->name, (ce)->name_length); \
		if (!memcmp((mname), lowercase_name, (mname_len))) { \
			if (!(ce)->constructor || (ce)->constructor == (orig_fe)) { \
				(ce)->constructor = (fe); \
				(fe)->common.fn_flags |= ZEND_ACC_CTOR; \
			} \
		} \
		efree(lowercase_name); \
	} \
}

#define DEEPTRACE_DEL_MAGIC_METHOD(ce, fe) { \
	if ((ce)->constructor == (fe))			(ce)->constructor	= NULL; \
	else if ((ce)->destructor == (fe))		(ce)->destructor	= NULL; \
	else if ((ce)->clone == (fe))			(ce)->clone			= NULL; \
	else if ((ce)->__get == (fe))			(ce)->__get			= NULL; \
	else if ((ce)->__set == (fe))			(ce)->__set			= NULL; \
	else if ((ce)->__unset == (fe))			(ce)->__unset		= NULL; \
	else if ((ce)->__isset == (fe))			(ce)->__isset		= NULL; \
	else if ((ce)->__call == (fe))			(ce)->__call		= NULL; \
	else if ((ce)->__callstatic == (fe))	(ce)->__callstatic	= NULL; \
	else if ((ce)->__tostring == (fe))		(ce)->__tostring	= NULL; \
}

#define DEEPTRACE_INHERIT_MAGIC_METHOD(ce, fe, orig_fe) { \
		if ((ce)->__get == (orig_fe) && (ce)->parent->__get == (fe)) { \
			(ce)->__get = (ce)->parent->__get; \
		} else if ((ce)->__set == (orig_fe) && (ce)->parent->__set == (fe)) { \
			(ce)->__set = (ce)->parent->__set; \
		} else if ((ce)->__unset == (orig_fe) && (ce)->parent->__unset == (fe)) { \
			(ce)->__unset = (ce)->parent->__unset; \
		} else if ((ce)->__isset == (orig_fe) && (ce)->parent->__isset == (fe)) { \
			(ce)->__isset = (ce)->parent->__isset; \
		} else if ((ce)->__call == (orig_fe) && (ce)->parent->__call == (fe)) { \
			(ce)->__call = (ce)->parent->__call; \
		} else if ((ce)->__callstatic == (orig_fe) && (ce)->parent->__callstatic == (fe)) { \
			(ce)->__callstatic = (ce)->parent->__callstatic; \
		} else if ((ce)->__tostring == (orig_fe) && (ce)->parent->__tostring == (fe)) { \
			(ce)->__tostring = (ce)->parent->__tostring; \
		} else if ((ce)->clone == (orig_fe) && (ce)->parent->clone == (fe)) { \
			(ce)->clone = (ce)->parent->clone; \
		} else if ((ce)->destructor == (orig_fe) && (ce)->parent->destructor == (fe)) { \
			(ce)->destructor = (ce)->parent->destructor; \
		} else if ((ce)->constructor == (orig_fe) && (ce)->parent->constructor == (fe)) { \
			(ce)->constructor = (ce)->parent->constructor; \
		} \
	}

/* DeepTrace internal functions */
int DeepTrace_exit_handler(ZEND_OPCODE_HANDLER_ARGS);
void DeepTrace_exit_cleanup();
void DeepTrace_functions_cleanup();
void DeepTrace_clear_all_functions_runtime_cache(TSRMLS_D);

/* DeepTrace PHP functions */
PHP_FUNCTION(dt_phpinfo_mode);
PHP_FUNCTION(dt_set_proctitle);
PHP_FUNCTION(dt_exit_mode);
PHP_FUNCTION(dt_exit_fetch_exception);
PHP_FUNCTION(dt_inspect_zval);
PHP_FUNCTION(dt_remove_include);
PHP_FUNCTION(dt_remove_function);
PHP_FUNCTION(dt_rename_function);
PHP_FUNCTION(dt_destroy_function_data);
PHP_FUNCTION(dt_set_static_function_variable);
PHP_FUNCTION(dt_remove_class);
PHP_FUNCTION(dt_destroy_class_data);
PHP_FUNCTION(dt_remove_constant);
PHP_FUNCTION(dt_add_method);
PHP_FUNCTION(dt_rename_method);
PHP_FUNCTION(dt_remove_method);
PHP_FUNCTION(dt_set_static_method_variable);

#if ZEND_DEBUG
PHP_FUNCTION(dt_debug_objects_store);
#endif

#endif
