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

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif
#include "php_DeepTrace.h"

/* Declare module globals */
ZEND_DECLARE_MODULE_GLOBALS(DeepTrace);
ZEND_EXTENSION();

/* DeepTrace argument info */
#ifdef DEEPTRACE_EXIT_MANIPULATION
	ZEND_BEGIN_ARG_INFO(arginfo_dt_set_exit_handler, 0)
		ZEND_ARG_INFO(0, "handlerFunction")
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO(arginfo_dt_throw_exit_exception, 0)
		ZEND_ARG_INFO(0, "throwException")
		ZEND_ARG_INFO(0, "exceptionType")
	ZEND_END_ARG_INFO()
#endif

#ifdef DEEPTRACE_CONSTANT_MANIPULATION
	ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_constant, 0)
		ZEND_ARG_INFO(0, "constantName")
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO(arginfo_dt_get_cache_size, 0)
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO(arginfo_dt_clear_cache, 0)
	ZEND_END_ARG_INFO()
#endif

#ifdef DEEPTRACE_THREAD_SUPPORT
	ZEND_BEGIN_ARG_INFO(arginfo_dt_set_proctitle, 0)
		ZEND_ARG_INFO(0, "processTitle")
	ZEND_END_ARG_INFO()
#endif

#ifdef DEEPTRACE_INFO_MANIPULATION
	ZEND_BEGIN_ARG_INFO(arginfo_dt_show_plain_info, 0)
		ZEND_ARG_INFO(0, "textOnly")
	ZEND_END_ARG_INFO()
#endif

#ifdef DEEPTRACE_INCLUDE_MANIPULATION
	ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_include, 0)
		ZEND_ARG_INFO(0, "includeName")
	ZEND_END_ARG_INFO()
#endif

#ifdef DEEPTRACE_FUNCTION_MANIPULATION
	ZEND_BEGIN_ARG_INFO(arginfo_dt_rename_function, 0)
		ZEND_ARG_INFO(0, "oldFunctionName")
		ZEND_ARG_INFO(0, "newFunctionName")
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_function, 0)
		ZEND_ARG_INFO(0, "functionName")
	ZEND_END_ARG_INFO()
#endif

#ifdef DEEPTRACE_CUSTOM_SUPERGLOBALS
	ZEND_BEGIN_ARG_INFO(arginfo_dt_get_superglobals, 0)
	ZEND_END_ARG_INFO()
#endif

#ifdef DEEPTRACE_CLASS_MANIPULATION
	ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_class, 0)
		ZEND_ARG_INFO(0, "className")
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_interface, 0)
		ZEND_ARG_INFO(0, "interfaceName")
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_trait, 0)
		ZEND_ARG_INFO(0, "traitName")
	ZEND_END_ARG_INFO()
#endif

/* DeepTrace function declaration */
const zend_function_entry DeepTrace_functions[] = {
	/* Exit manipulation */
#	ifdef DEEPTRACE_EXIT_MANIPULATION
		PHP_FE(dt_set_exit_handler, arginfo_dt_set_exit_handler)
		PHP_FE(dt_throw_exit_exception, arginfo_dt_throw_exit_exception)
#	endif
	
	/* Thread support */
#	ifdef DEEPTRACE_THREAD_SUPPORT
		PHP_FE(dt_set_proctitle, arginfo_dt_set_proctitle)
#	endif

	/* Info manipulation */
#	ifdef DEEPTRACE_INFO_MANIPULATION
		PHP_FE(dt_show_plain_info, arginfo_dt_show_plain_info)
#	endif

	/* Include manipulation */
#	ifdef DEEPTRACE_INCLUDE_MANIPULATION
		PHP_FE(dt_remove_include, arginfo_dt_remove_include)
#	endif

	/* Function manipulation */
#	ifdef DEEPTRACE_FUNCTION_MANIPULATION
		PHP_FE(dt_rename_function, arginfo_dt_rename_function)
		PHP_FE(dt_remove_function, arginfo_dt_remove_function)
#	endif

	/* Class manipulation */
#	ifdef DEEPTRACE_CLASS_MANIPULATION
		PHP_FE(dt_remove_class, arginfo_dt_remove_class)
		PHP_FE(dt_remove_interface, arginfo_dt_remove_interface)
		PHP_FE(dt_remove_trait, arginfo_dt_remove_trait)
#	endif

	/* Constant manipulation */
#	ifdef DEEPTRACE_CONSTANT_MANIPULATION
		PHP_FE(dt_remove_constant, arginfo_dt_remove_constant)
#		if DT_PHP_VERSION == 54
#			ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
				PHP_FE(dt_get_cache_size, arginfo_dt_get_cache_size)
				PHP_FE(dt_clear_cache, arginfo_dt_clear_cache)
#			endif
#		endif
#	endif

	/* Custom superglobals */
#	ifdef DEEPTRACE_CUSTOM_SUPERGLOBALS
		PHP_FE(dt_get_superglobals, arginfo_dt_get_superglobals)
#	endif

	PHP_FE_END
};

/* {{{ DeepTrace_init_globals
	Init DeepTrace globals */
void DeepTrace_init_globals(zend_DeepTrace_globals *globals)
{
	/* Thread support */
#	ifdef DEEPTRACE_THREAD_SUPPORT
		globals->argv0 = NULL;
#	endif

	/* Info manipulation */
#	ifdef DEEPTRACE_INFO_MANIPULATION
		globals->infoMode = sapi_module.phpinfo_as_text;
#	endif

	/* Exit manipulation */
#	ifdef DEEPTRACE_EXIT_MANIPULATION	
		globals->exitHandler.fci.function_name = NULL;
		globals->exitHandler.fci.object_ptr = NULL;
		globals->throwException = 0;
		globals->exitExceptionType = DEEPTRACE_EXIT_EXCEPTION_TYPE;
		globals->exitException = NULL;
		globals->exitExceptionClass.name = NULL;
#	endif

	/* Function manipulation */
#	ifdef DEEPTRACE_FUNCTION_MANIPULATION
		globals->replaced_internal_functions = NULL;
		globals->misplaced_internal_functions = NULL;
#	endif

	/* Constant manipulation */
#	ifdef DEEPTRACE_CONSTANT_MANIPULATION
#		if DT_PHP_VERSION == 54
#			ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
				globals->constantCache = NULL;
#			endif
#		endif
#	endif
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION(DeepTrace)
	DeepTrace - Module init function */
PHP_MINIT_FUNCTION(DeepTrace)
{
	/* Init module globals */
	ZEND_INIT_MODULE_GLOBALS(DeepTrace, DeepTrace_init_globals, NULL);

	/* Get pointer to argv0 */
#	ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
		sapi_module_struct *symbol = &sapi_module;
		if(symbol) DEEPTRACE_G(argv0) = symbol->executable_location;
#	endif

	/* Register exit opcode handler */
#	ifdef DEEPTRACE_EXIT_MANIPULATION
		oldExitHandler = zend_get_user_opcode_handler(ZEND_EXIT);
		zend_set_user_opcode_handler(ZEND_EXIT, DeepTrace_exit_handler);
#	endif

	/* Run Time Cache Bypass */
#	ifdef DEEPTRACE_CONSTANT_MANIPULATION
#		if DT_PHP_VERSION == 54
#			ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
				zend_set_user_opcode_handler(ZEND_FETCH_CONSTANT, DeepTrace_constant_handler);
#			endif
#		endif
#	endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION(DeepTrace)
	DeepTrace - Request init function */
PHP_RINIT_FUNCTION(DeepTrace)
{
	/* Set hashtables to NULL */
#	ifdef DEEPTRACE_FUNCTION_MANIPULATION
		DEEPTRACE_G(replaced_internal_functions) = NULL;
		DEEPTRACE_G(misplaced_internal_functions) = NULL;
#	endif
#	ifdef DEEPTRACE_CONSTANT_MANIPULATION
#		if DT_PHP_VERSION == 54
#			ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
				DEEPTRACE_G(constantCache) = NULL;
#			endif
#		endif
#	endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(DeepTrace)
	DeepTrace - Request shutdown function */
PHP_RSHUTDOWN_FUNCTION(DeepTrace)
{
	/* Unload exit handler */
#	ifdef DEEPTRACE_EXIT_MANIPULATION
		DeepTrace_free_handler(&DEEPTRACE_G(exitHandler).fci);
		zend_set_user_opcode_handler(ZEND_EXIT, NULL);
#	endif

	/* Unload constant manipulation */
#	ifdef DEEPTRACE_CONSTANT_MANIPULATION
#		if DT_PHP_VERSION == 54
#			ifdef DEEPTRACE_FIX_RUN_TIME_CACHE
				/* Delete runtime cache hashtable */
				if(DEEPTRACE_G(constantCache)) {
					zend_hash_apply(DEEPTRACE_G(constantCache), (apply_func_t) DeepTrace_destroy_cache_entries TSRMLS_CC);
					zend_hash_destroy(DEEPTRACE_G(constantCache));
					FREE_HASHTABLE(DEEPTRACE_G(constantCache));
					DEEPTRACE_G(constantCache) = NULL;
				}
#			endif
#		endif
#	endif

	/* Fix internal functions */
#	ifdef DEEPTRACE_FUNCTION_MANIPULATION
		if(DEEPTRACE_G(misplaced_internal_functions)) {
			zend_hash_apply(DEEPTRACE_G(misplaced_internal_functions), (apply_func_t) DeepTrace_destroy_misplaced_functions TSRMLS_CC);
			zend_hash_destroy(DEEPTRACE_G(misplaced_internal_functions));
			FREE_HASHTABLE(DEEPTRACE_G(misplaced_internal_functions));
			DEEPTRACE_G(misplaced_internal_functions) = NULL;
		}

		if(DEEPTRACE_G(replaced_internal_functions)) {
			zend_hash_apply_with_arguments(DEEPTRACE_G(replaced_internal_functions) TSRMLS_CC, (apply_func_args_t) DeepTrace_restore_internal_functions, 1, DEEPTRACE_TSRMLS_C);
			zend_hash_destroy(DEEPTRACE_G(replaced_internal_functions));
			FREE_HASHTABLE(DEEPTRACE_G(replaced_internal_functions));
			DEEPTRACE_G(replaced_internal_functions) = NULL;
		}

		zend_hash_apply(EG(function_table), DeepTrace_delete_user_functions TSRMLS_CC);
#	endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(DeepTrace)
	DeepTrace - Module exit function */
PHP_MSHUTDOWN_FUNCTION(DeepTrace)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION(DeepTrace)
	DeepTrace - phpinfo function */
PHP_MINFO_FUNCTION(DeepTrace)
{
	/* You will violate the CC BY-NC-SA 3.0 license
	if you change any information here! */
	php_info_print_box_start(0);
	php_printf("DeepTrace v%s - Copyright (c) 2012 P. Mathis<br />\n", DEEPTRACE_VERSION);
	php_printf("https://www.snapserv.net/<br />\n");
	php_printf("Licensed under CC BY-NC-SA 3.0<br />\n");
	php_info_print_box_end();

	php_info_print_table_start();
	php_info_print_table_header(2, "DeepTrace support", "enabled");
	php_info_print_table_row(2, "Version", DEEPTRACE_VERSION);
	php_info_print_table_row(2, "Build Date", __DATE__);
	php_info_print_table_row(2, "Build Time", __TIME__);
	php_info_print_table_end();
}
/* }}} */

/* {{{ DeepTrace_zend_startup
	Zend startup function */
int DeepTrace_zend_startup(zend_extension *extension)
{
#	ifdef DEEPTRACE_CONSTANT_MANIPULATION
		/* Disable constant substitution */
		CG(compiler_options) |= ZEND_COMPILE_NO_CONSTANT_SUBSTITUTION;
#	endif

	return zend_startup_module(&DeepTrace_module_entry);
}
/* }}} */

/* DeepTrace module entry */
zend_module_entry DeepTrace_module_entry = {
	STANDARD_MODULE_HEADER,
	"DeepTrace",
	DeepTrace_functions,
	PHP_MINIT(DeepTrace),
	PHP_MSHUTDOWN(DeepTrace),
	PHP_RINIT(DeepTrace),
	PHP_RSHUTDOWN(DeepTrace),
	PHP_MINFO(DeepTrace),
	DEEPTRACE_VERSION,
	STANDARD_MODULE_PROPERTIES
};

/* DeepTrace zend extension entry */
zend_extension zend_extension_entry = {
	"DeepTrace",
	DEEPTRACE_VERSION,
	"Pascal Mathis",
	"https://www.snapserv.net",
	"Copyright (c) 2012",
	DeepTrace_zend_startup,
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,
	STANDARD_ZEND_EXTENSION_PROPERTIES
};