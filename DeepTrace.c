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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php_DeepTrace.h"

/* Declare DeepTrace module globals */
ZEND_DECLARE_MODULE_GLOBALS(DeepTrace);

/* {{{ DeepTrace argument info table */
ZEND_BEGIN_ARG_INFO(arginfo_dt_phpinfo_mode, 0)
	ZEND_ARG_INFO(0, "mode")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_set_proctitle, 0)
	ZEND_ARG_INFO(0, "processTitle")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_exit_mode, 0)
	ZEND_ARG_INFO(0, "exitMode")
	ZEND_ARG_INFO(0, "exitHandler")
	ZEND_ARG_INFO(0, "exitExceptionName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_exit_fetch_exception, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_include, 0)
	ZEND_ARG_INFO(0, "includeName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_function, 0)
	ZEND_ARG_INFO(0, "functionName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_rename_function, 0)
	ZEND_ARG_INFO(0, "oldFunctionName")
	ZEND_ARG_INFO(0, "newFunctionName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_destroy_function_data, 0)
	ZEND_ARG_INFO(0, "functionName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_inspect_zval, 0)
	ZEND_ARG_INFO(0, "zval")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_set_static_function_variable, 0)
	ZEND_ARG_INFO(0, "functionName")
	ZEND_ARG_INFO(0, "variableName")
	ZEND_ARG_INFO(0, "newValue")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_class, 0)
	ZEND_ARG_INFO(0, "className")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_destroy_class_data, 0)
	ZEND_ARG_INFO(0, "className")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_constant, 0)
	ZEND_ARG_INFO(0, "constantName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_add_method, 0)
	ZEND_ARG_INFO(0, "className")
	ZEND_ARG_INFO(0, "methodName")
	ZEND_ARG_INFO(0, "arguments")
	ZEND_ARG_INFO(0, "phpcode")
	ZEND_ARG_INFO(0, "flags")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_rename_method, 0)
	ZEND_ARG_INFO(0, "className")
	ZEND_ARG_INFO(0, "oldMethodName")
	ZEND_ARG_INFO(0, "newMethodName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_method, 0)
	ZEND_ARG_INFO(0, "className")
	ZEND_ARG_INFO(0, "methodName")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_dt_set_static_method_variable, 0)
	ZEND_ARG_INFO(0, "className")
	ZEND_ARG_INFO(0, "methodName")
	ZEND_ARG_INFO(0, "variableName")
	ZEND_ARG_INFO(0, "newValue")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ DeepTrace function table */
const zend_function_entry DeepTrace_functions[] = {
		PHP_FE(dt_phpinfo_mode, arginfo_dt_phpinfo_mode)
#if DEEPTRACE_PHP_VERSION < 55
		PHP_FE(dt_set_proctitle, arginfo_dt_set_proctitle)
#endif
		PHP_FE(dt_remove_include, arginfo_dt_remove_include)
		PHP_FE(dt_inspect_zval, arginfo_dt_inspect_zval)

		PHP_FE(dt_exit_mode, arginfo_dt_exit_mode)
		PHP_FE(dt_exit_fetch_exception, arginfo_dt_exit_fetch_exception)

		PHP_FE(dt_remove_function, arginfo_dt_remove_function)
		PHP_FE(dt_rename_function, arginfo_dt_rename_function)
		PHP_FE(dt_destroy_function_data, arginfo_dt_destroy_function_data)
		PHP_FE(dt_set_static_function_variable, arginfo_dt_set_static_function_variable)

		PHP_FE(dt_remove_class, arginfo_dt_remove_class)
		PHP_FE(dt_destroy_class_data, arginfo_dt_destroy_class_data)

		PHP_FE(dt_remove_constant, arginfo_dt_remove_constant)

		PHP_FE(dt_add_method, arginfo_dt_add_method)
		PHP_FE(dt_rename_method, arginfo_dt_rename_method)
		PHP_FE(dt_remove_method, arginfo_dt_remove_method)
		PHP_FE(dt_set_static_method_variable, arginfo_dt_set_static_method_variable)
#if ZEND_DEBUG
		PHP_FE(dt_debug_objects_store, NULL)
#endif
		PHP_FE_END
};
/* }}} */

/* {{{ DeepTrace_init_globals */
void DeepTrace_init_globals(zend_DeepTrace_globals *globals)
{
	globals->argv0 = NULL;

	globals->exitMode = DEEPTRACE_EXIT_NORMAL;
	globals->exitOldHandler = NULL;
	globals->exitHandler.fci.function_name = NULL;
	globals->exitHandler.fci.object_ptr = NULL;
	globals->exitException = NULL;

	globals->misplaced_internal_functions = NULL;
	globals->replaced_internal_functions = NULL;
	globals->misplaced_internal_methods = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION(DeepTrace) */
PHP_MINIT_FUNCTION(DeepTrace)
{
	ZEND_INIT_MODULE_GLOBALS(DeepTrace, DeepTrace_init_globals, NULL);

	/* Register constants */
	REGISTER_LONG_CONSTANT("DT_PHPINFO_HTML", DEEPTRACE_PHPINFO_HTML, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DT_PHPINFO_TEXT", DEEPTRACE_PHPINFO_TEXT, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("DT_EXIT_NORMAL", DEEPTRACE_EXIT_NORMAL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DT_EXIT_HANDLER", DEEPTRACE_EXIT_HANDLER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DT_EXIT_EXCEPTION", DEEPTRACE_EXIT_EXCEPTION, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("DT_ACC_PUBLIC", ZEND_ACC_PUBLIC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DT_ACC_PROTECTED", ZEND_ACC_PROTECTED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DT_ACC_PRIVATE", ZEND_ACC_PRIVATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DT_ACC_STATIC", ZEND_ACC_STATIC, CONST_CS | CONST_PERSISTENT);

	/* Get pointer to argv0 */
	sapi_module_struct *symbol = &sapi_module;
	if(symbol) {
		DEEPTRACE_G(argv0) = symbol->executable_location;
	}

	/* Register opcode handlers */
	DEEPTRACE_G(exitOldHandler) = zend_get_user_opcode_handler(ZEND_EXIT);
	zend_set_user_opcode_handler(ZEND_EXIT, DeepTrace_exit_handler);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(DeepTrace) */
PHP_MSHUTDOWN_FUNCTION(DeepTrace)
{
	zend_set_user_opcode_handler(ZEND_EXIT, NULL);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION(DeepTrace) */
PHP_RINIT_FUNCTION(DeepTrace)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION(DeepTrace) */
PHP_RSHUTDOWN_FUNCTION(DeepTrace)
{
	DeepTrace_exit_cleanup(TSRMLS_C);
	DeepTrace_functions_cleanup(TSRMLS_C);
	DeepTrace_methods_cleanup(TSRMLS_C);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION(DeepTrace) */
PHP_MINFO_FUNCTION(DeepTrace)
{
	if(sapi_module.phpinfo_as_text) {
		php_info_print_box_start(0);
		php_printf("DeepTrace v%s - Copyright (c) 2012-2013\n", DEEPTRACE_VERSION);
		php_printf("&lt; P. Mathis <pmathis@snapserv.net>\n");
		php_printf("&lt; Y. Khalil <dev@pp3345.net>\n\n");
		php_printf("Licensed under Apache License v2");
		php_info_print_box_end();
	} else {
		php_info_print_box_start(0);
		php_printf("DeepTrace v%s - Copyright (c) 2012-2013<br />", DEEPTRACE_VERSION);
		php_printf("&gt; P. Mathis &lt;pmathis@snapserv.net&gt;<br />");
		php_printf("&gt; Y. Khalil &lt;dev@pp3345.net&gt;<br /><br />");
		php_printf("Licensed under Apache License v2");
		php_info_print_box_end();
	}

	php_info_print_table_start();
	php_info_print_table_header(2, "DeepTrace support", "enabled");
	php_info_print_table_row(2, "Version", DEEPTRACE_VERSION);
	php_info_print_table_row(2, "Build Date", __DATE__);
	php_info_print_table_row(2, "Build Time", __TIME__);
	php_info_print_table_end();
}
/* }}} */

/* {{{ PHP module entry for DeepTrace extension */
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
/* }}} */

/* {{{ DeepTrace_zend_startup */
int DeepTrace_zend_startup(zend_extension *extension) {
	TSRMLS_FETCH();
	CG(compiler_options) |= ZEND_COMPILE_NO_CONSTANT_SUBSTITUTION;

	return zend_startup_module(&DeepTrace_module_entry);
}
/* }}} */

/* {{{ Zend extension entry for DeepTrace extension */
#ifndef ZEND_EXT_API
#define ZEND_EXT_API ZEND_DLEXPORT
#endif
ZEND_EXTENSION();

zend_extension zend_extension_entry = {
		"DeepTrace",
		DEEPTRACE_VERSION,
		"Pascal Mathis & Yussuf Khalil",
		"http://www.snapserv.net",
		"Copyright (c) 2012-2013",
		DeepTrace_zend_startup,
		NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL,
		STANDARD_ZEND_EXTENSION_PROPERTIES
};

#ifdef COMPILE_DL_DEEPTRACE
ZEND_GET_MODULE(DeepTrace)
#endif
/* }}} */
