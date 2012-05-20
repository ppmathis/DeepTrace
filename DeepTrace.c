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

// Declare module globals
ZEND_DECLARE_MODULE_GLOBALS(DeepTrace);
ZEND_EXTENSION();

// DeepTrace public function arguments
ZEND_BEGIN_ARG_INFO(arginfo_dt_set_exit_handler, 0)
	ZEND_ARG_INFO(0, "handlerFunction")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_throw_exit_exception, 0)
	ZEND_ARG_INFO(0, "throwException")
	ZEND_ARG_INFO(0, "exceptionType")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_constant, 0)
	ZEND_ARG_INFO(0, "constantName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_set_proctitle, 0)
	ZEND_ARG_INFO(0, "processTitle")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_show_plain_info, 0)
	ZEND_ARG_INFO(0, "infoMode")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_include, 0)
	ZEND_ARG_INFO(0, "includeName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_rename_function, 0)
	ZEND_ARG_INFO(0, "oldFunctionName")
	ZEND_ARG_INFO(0, "newFunctionName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_function, 0)
	ZEND_ARG_INFO(0, "functionName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_class, 0)
	ZEND_ARG_INFO(0, "className")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_interface, 0)
	ZEND_ARG_INFO(0, "interfaceName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_trait, 0)
	ZEND_ARG_INFO(0, "traitName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_get_cache_size, 0)
ZEND_END_ARG_INFO()

// DeepTrace public functions
const zend_function_entry DeepTrace_functions[] = {
	PHP_FE(dt_set_exit_handler, arginfo_dt_set_exit_handler)
	PHP_FE(dt_throw_exit_exception, arginfo_dt_throw_exit_exception)
	PHP_FE(dt_remove_constant, arginfo_dt_remove_constant)
	PHP_FE(dt_set_proctitle, arginfo_dt_set_proctitle)
	PHP_FE(dt_show_plain_info, arginfo_dt_show_plain_info)
	PHP_FE(dt_remove_include, arginfo_dt_remove_include)
	PHP_FE(dt_rename_function, arginfo_dt_rename_function)
	PHP_FE(dt_remove_function, arginfo_dt_remove_function)
	PHP_FE(dt_remove_class, arginfo_dt_remove_class)
	PHP_FE(dt_remove_interface, arginfo_dt_remove_interface)
	PHP_FE(dt_remove_trait, arginfo_dt_remove_trait)
	PHP_FE(dt_get_cache_size, arginfo_dt_get_cache_size)
	PHP_FE_END
};

// DeepTrace_init_globals
void DeepTrace_init_globals(zend_DeepTrace_globals *globals)
{
	globals->argv0 = NULL;
	globals->infoMode = sapi_module.phpinfo_as_text;
	globals->exitHandler.fci.function_name = NULL;
	globals->exitHandler.fci.object_ptr = NULL;
	globals->throwException = 0;
	globals->exitExceptionType = DEEPTRACE_EXIT_EXCEPTION_TYPE;
	globals->exitException = NULL;
	globals->exitExceptionClass.name = NULL;
	globals->replaced_internal_functions = NULL;
	globals->misplaced_internal_functions = NULL;
	globals->constantCache = NULL;
}

// DeepTrace MINIT function
PHP_MINIT_FUNCTION(DeepTrace)
{
	// Init module globals
	ZEND_INIT_MODULE_GLOBALS(DeepTrace, DeepTrace_init_globals, NULL);

	// Get argv0
	sapi_module_struct *symbol = &sapi_module;
	if(symbol) DEEPTRACE_G(argv0) = symbol->executable_location;

	// Register exit opcode handler
	oldExitHandler = zend_get_user_opcode_handler(ZEND_EXIT);
	zend_set_user_opcode_handler(ZEND_EXIT, DeepTrace_exit_handler);

	// Run Time Cache Bypass
	zend_set_user_opcode_handler(ZEND_FETCH_CONSTANT, DeepTrace_constant_handler);

	return SUCCESS;
}

// DeepTrace RINIT function
PHP_RINIT_FUNCTION(DeepTrace)
{
	// Clear tables
	DEEPTRACE_G(replaced_internal_functions) = NULL;
	DEEPTRACE_G(misplaced_internal_functions) = NULL;
	DEEPTRACE_G(constantCache) = NULL;

	return SUCCESS;
}

// DeepTrace RSHUTDOWN function
PHP_RSHUTDOWN_FUNCTION(DeepTrace)
{
	// Unload handlers
	DeepTrace_free_handler(&DEEPTRACE_G(exitHandler).fci);
	zend_set_user_opcode_handler(ZEND_EXIT, NULL);

	// Kill runtimecache hashtable
	zend_hash_destroy(DEEPTRACE_G(constantCache));
	FREE_HASHTABLE(DEEPTRACE_G(constantCache));
	DEEPTRACE_G(constantCache) = NULL;

	// Fix internal functions
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
	return SUCCESS;
}

// DeepTrace MSHUTDOWN function
PHP_MSHUTDOWN_FUNCTION(DeepTrace)
{
	return SUCCESS;
}

// DeepTrace MINFO function
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

// DeepTrace zend startup function
int DeepTrace_zend_startup(zend_extension *extension)
{
	// Disable constant substitution
	CG(compiler_options) |= ZEND_COMPILE_NO_CONSTANT_SUBSTITUTION;
	return zend_startup_module(&DeepTrace_module_entry);
}

// DeepTrace module entry
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

// DeepTrace zend extension entry
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