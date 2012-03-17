/*
	+-----------------------------------------------------------------------+
	| DeepTrace v1.2.1 ( Homepage: https://www.snapserv.net/ )			 	|
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

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_DeepTrace.h"
#include "Zend/zend_constants.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_extensions.h"
#include "dlfcn.h"
#include "SAPI.h"

/* True global resources - no need for thread safety here */
static int le_DeepTrace;
static char *argv0 = NULL;

typedef struct {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} user_handler_t;

static user_opcode_handler_t old_exit_handler = NULL;
ZEND_BEGIN_MODULE_GLOBALS(DeepTrace)
	int throwException;
	int infoMode;
	user_handler_t exit_handler;
ZEND_END_MODULE_GLOBALS(DeepTrace)
ZEND_DECLARE_MODULE_GLOBALS(DeepTrace)

/* Macros */
#ifdef ZTS
#define THG(v) TSRMG(DeepTrace_globals_id, zend_DeepTrace_globals *, v)
#else
#define THG(v) (DeepTrace_globals.v)
#endif

#undef EX
#define EX(element) execute_data->element
#define EX_T(offset) (*(temp_variable *)((char *) EX(Ts) + offset))

#define DEEPTRACE_VERSION "1.2.1"
#define DEEPTRACE_PROCTITLE_MAX_LEN 128

/* {{{ Get ZVAL ptr
*/
#if PHP_API_VERSION > 20090626
	// PHP 5.4
	static zval *pth_get_zval_ptr(int op_type, znode_op *node, zval **freeval, zend_execute_data *execute_data TSRMLS_DC) /* {{{ */
	{
		*freeval = NULL;

		switch (op_type) {
		case IS_CONST:
			return node->zv;
		case IS_VAR:
			return EX_T(node->var).var.ptr;
		case IS_TMP_VAR:
			return (*freeval = &EX_T(node->var).tmp_var);
		case IS_CV:
			{
			zval ***ret = &execute_data->CVs[node->var];
			if (!*ret) {
					zend_compiled_variable *cv = &EG(active_op_array)->vars[node->var];
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, (void**)ret)==FAILURE) {
						zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
						return &EG(uninitialized_zval);
					}
			}
			return **ret;
			}
		case IS_UNUSED:
		default:
			return NULL;
		}
	}
#else
	// PHP 5.3
	static zval *pth_get_zval_ptr(znode *node, zval **freeval, zend_execute_data *execute_data TSRMLS_DC) /* {{{ */
	{
		*freeval = NULL;

		switch (node->op_type) {
		case IS_CONST:
			return &(node->u.constant);
		case IS_VAR:
			return EX_T(node->u.var).var.ptr;
		case IS_TMP_VAR:
			return (*freeval = &EX_T(node->u.var).tmp_var);
		case IS_CV:
			{
			zval ***ret = &execute_data->CVs[node->u.var];
			if (!*ret) {
					zend_compiled_variable *cv = &EG(active_op_array)->vars[node->u.var];
					if (zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, (void**)ret)==FAILURE) {
						zend_error(E_NOTICE, "Undefined variable: %s", cv->name);
						return &EG(uninitialized_zval);
					}
			}
			return **ret;
			}
		case IS_UNUSED:
		default:
			return NULL;
		}
	}
#endif
/* }}} */

/* {{{ DeepTrace init globals
*/
static void php_DeepTrace_init_globals(zend_DeepTrace_globals *globals)
{
	globals->throwException = 0;
	globals->exit_handler.fci.function_name = NULL;
	globals->exit_handler.fci.object_ptr = NULL;
}
/* }}} */

/* {{{ DeepTrace exit handler
*/
static int DeepTrace_exit_handler(ZEND_OPCODE_HANDLER_ARGS)
{
	zval *msg, *freeop;
	zval *retval;
	
	if(THG(exit_handler).fci.function_name == NULL) {
		if(old_exit_handler) {
			return old_exit_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		} else {
			return ZEND_USER_OPCODE_DISPATCH;
		}
	}
	
	#if PHP_API_VERSION > 20090626
		// PHP 5.4
		msg = pth_get_zval_ptr(EX(opline)->op1_type, &EX(opline)->op1, &freeop, execute_data TSRMLS_CC);
	#else
		// PHP 5.3
		msg = pth_get_zval_ptr(&EX(opline)->op1, &freeop, execute_data TSRMLS_CC);
	#endif
	if(msg) {
		zend_fcall_info_argn(&THG(exit_handler).fci TSRMLS_CC, 1, &msg);
	}
	
	zend_fcall_info_call(&THG(exit_handler).fci, &THG(exit_handler).fcc, &retval, NULL TSRMLS_CC);
	zend_fcall_info_args_clear(&THG(exit_handler).fci, 1);

	// Ignore that fucking exit()
	convert_to_boolean(retval);
	if(Z_LVAL_P(retval)) {
		zval_ptr_dtor(&retval);
		if(old_exit_handler) {
			return old_exit_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		} else {
			return ZEND_USER_OPCODE_DISPATCH;
		}
	} else {
		zval_ptr_dtor(&retval);
		EX(opline)++;
		
		// Throw exception if necessary
		if(THG(throwException) == 1) {
			zend_throw_exception(zend_exception_get_default(TSRMLS_C), "DeepTraceExitException", 0 TSRMLS_CC);
		}
		
		// Continue
		return ZEND_USER_OPCODE_CONTINUE;
	}
}
/* }}} */

/* {{{ DeepTrace proctitle */
static void DeepTrace_set_proctitle(char *title, int len) {
	char buffer[DEEPTRACE_PROCTITLE_MAX_LEN];
	
	// Only if argv0 exists
	if(!argv0) {
		return;
	}
	
	// Copy to argv0
	memset(buffer, 0x20, DEEPTRACE_PROCTITLE_MAX_LEN);
	if(len >= (DEEPTRACE_PROCTITLE_MAX_LEN - 1)) {
		len = DEEPTRACE_PROCTITLE_MAX_LEN - 1;
	}
	buffer[DEEPTRACE_PROCTITLE_MAX_LEN - 1] = '\0';
	memcpy(buffer, title, len);
	snprintf(argv0, DEEPTRACE_PROCTITLE_MAX_LEN, "%s", buffer);
}
/* }}} */

/* {{{ DeepTrace free handler
*/
static void DeepTrace_free_handler(zend_fcall_info *fci)
{
	if(fci->function_name) {
		zval_ptr_dtor(&fci->function_name);
		fci->function_name = NULL;
	}
	if(fci->object_ptr) {
		zval_ptr_dtor(&fci->object_ptr);
		fci->object_ptr = NULL;
	}
}
/* }}} */

/* {{{ DeepTrace set handler
*/
static void DeepTrace_set_handler(user_opcode_handler_t op_handler, int opcode, user_handler_t* handler, INTERNAL_FUNCTION_PARAMETERS)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	
	// Check parameters
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &fci, &fcc) == FAILURE) {
		return;
	}
	
	// Is extension working?
	if(op_handler != zend_get_user_opcode_handler(opcode)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Conflicting extension detected. Make sure to load DeepTrace as zend extension after other extensions.");	
	}
	
	handler->fci = fci;
	handler->fcc = fcc;
	Z_ADDREF_P(handler->fci.function_name);
	if(handler->fci.object_ptr) {
		Z_ADDREF_P(handler->fci.object_ptr);
	}
	
	RETURN_TRUE;
}

/* {{{ proto bool dt_set_exit_handler(callback cb)
	Register a callback, called on exit()/die() */
static PHP_FUNCTION(dt_set_exit_handler)
{
	DeepTrace_set_handler(DeepTrace_exit_handler, ZEND_EXIT, &THG(exit_handler), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void dt_set_exit_exception(string exception)
	Set throw string after exit()/die() callback */
static PHP_FUNCTION(dt_throw_exit_exception)
{
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &THG(throwException)) == FAILURE) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_set_proctitle(string proctitle)
	Set proctitle to string */
static PHP_FUNCTION(dt_set_proctitle)
{
	char* title;
	int len;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &title, &len) == FAILURE) {
		RETURN_FALSE;
	}
	
	// Use correct set proc title command
	#ifndef PHP_SYSTEM_PROVIDES_SETPROCTITLE
		DeepTrace_set_proctitle(title, len);
	#else
		setproctitle("%s", title);
	#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_remove_function(string function)
	Remove function from function table */
static PHP_FUNCTION(dt_remove_function)
{
	char* functionName;
	int len;

	// Get function name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &functionName, &len) == FAILURE) {
		RETURN_FALSE;
	}

	// Lowercase
	functionName = zend_str_tolower_dup(functionName, len);

	// Check function name
	if(!zend_hash_exists(EG(function_table), functionName, len + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Function %s does not exist.", functionName);
		RETURN_FALSE;
	}

	// Delete function
	zend_hash_del(EG(function_table), functionName, len + 1);

	// Free memory
	efree(functionName);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_rename_function(string oldFunction, string newFunction)
	Rename existing function */
static PHP_FUNCTION(dt_rename_function)
{
	zend_function *f, func;
	char *oldFunction, *newFunction;
	int oldLen, newLen;



	// Get function names
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &oldFunction, &oldLen, &newFunction, &newLen) == FAILURE) {
		RETURN_FALSE;
	}

	// Lowercase
	oldFunction = zend_str_tolower_dup(oldFunction, oldLen);
	newFunction = zend_str_tolower_dup(newFunction, newLen);

	// Check new function name
	if(zend_hash_exists(EG(function_table), newFunction, newLen + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "New function %s() already exists.", newFunction);
		efree(oldFunction);
		efree(newFunction);
		RETURN_FALSE;
	}

	// Check old function name
	if(zend_hash_find(EG(function_table), oldFunction, oldLen + 1, (void**) &f) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Old function %s() does not exist.", oldFunction);
		efree(oldFunction);
		efree(newFunction);
		RETURN_FALSE;
	}

	// Create function (with reference)
	func = (zend_function) *f;
	function_add_ref(&func);

	// Remove old reference
	if(zend_hash_del(EG(function_table), oldFunction, oldLen + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove old reference to function %s().", oldFunction);
		zend_function_dtor(&func);
		efree(oldFunction);
		efree(newFunction);
		RETURN_FALSE;
	}

	// Is user function?
	if(func.type == ZEND_USER_FUNCTION) {
		efree((void *) func.common.function_name);
		func.common.function_name = estrndup(newFunction, newLen);
	}

	// Add new reference
	if(zend_hash_add(EG(function_table), newFunction, newLen + 1, &func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not create new reference to function %s().", newFunction);
		zend_function_dtor(&func);
		efree(oldFunction);
		efree(newFunction);
		RETURN_FALSE;
	}		

	// Free memory
	efree(oldFunction);
	efree(newFunction);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_remove_constant(string constName)
	Remove constant */
static PHP_FUNCTION(dt_remove_constant)
{
	char* constName;
	char* lcase;
	int len;
	zend_constant *constant;

	// Get function name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &constName, &len) == FAILURE) {
		RETURN_FALSE;
	}

	// Get constant
	if(zend_hash_find(EG(zend_constants), constName, len + 1, (void**) &constant) == FAILURE) {
		lcase = zend_str_tolower_dup(constName, len);
		if(zend_hash_find(EG(zend_constants), lcase, len + 1, (void**) &constant) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Constant %s does not exist.", constName);
			efree(lcase);
			RETURN_FALSE;
		}
		efree(lcase);

		if((constant->flags & CONST_CS)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Constant %s is case-sensitive.", constant->name);
			efree(lcase);
			RETURN_FALSE;
		}
	}

	// Safety warning
	if(constant->flags & CONST_PERSISTENT) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Constant %s is persistent. You should not modify it.", constant->name);
	}

	// Case-Sensitive?
	if((constant->flags & CONST_CS) == 0) {
		constName = zend_str_tolower_dup(constName, len);
	} else {
		constName = constant->name;
	}

	// Delete constant
	if(zend_hash_del(EG(zend_constants), constName, len + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not remove constant %s.", constName);
		if((constant->flags & CONST_CS) == 0) {
			efree(constName);
		}
		RETURN_FALSE;
	}

	// Free memory
	if((constant->flags & CONST_CS) == 0) {
		efree(constName);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_remove_include(string includeName)
	Removes an include from the internal hash list (execute require_once multiple times) */
static PHP_FUNCTION(dt_remove_include)
{
	char *includeName;
	char *absolutePath;
	int len;

	// Get include name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &includeName, &len) == FAILURE) {
		RETURN_FALSE;
	}

	// Get real path
	absolutePath = zend_resolve_path(includeName, len);

	// Find include in hash map
	if(!zend_hash_exists(&EG(included_files), absolutePath, strlen(absolutePath) + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Include %s does not exist.", includeName);
		efree(absolutePath);
		RETURN_FALSE;
	}	

	// Remove include in hash map
	if(zend_hash_del(&EG(included_files), absolutePath, strlen(absolutePath) + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove include %s.", includeName);
		efree(absolutePath);
		RETURN_FALSE;
	}

	// Free memory
	efree(absolutePath);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_remove_class(string className)
	Removes a class */
static PHP_FUNCTION(dt_remove_class)
{
	char *className;
	char *lcase;
	int len;

	// Get class name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &className, &len) == FAILURE) {
		RETURN_FALSE;
	}

	// Make class name lower case
	lcase = zend_str_tolower_dup(className, len);

	// Check if class exists
	if(!zend_hash_exists(EG(class_table), lcase, len + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class %s does not exist.", className);
		efree(lcase);
		RETURN_FALSE;
	}

	// Remove class in hash map
	if(zend_hash_del(EG(class_table), lcase, len + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove class %s.", className);
		efree(lcase);
		RETURN_FALSE;
	}

	// Free memory
	efree(lcase);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_show_plain_info(bool state)
	Toggles phpinfo() mode (plain / html) */
static PHP_FUNCTION(dt_show_plain_info)
{
	// Get state
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &THG(infoMode)) == FAILURE) {
		RETURN_FALSE;
	}

	// Set state
	if(THG(infoMode)) {
		sapi_module.phpinfo_as_text = 1;
	} else {
		sapi_module.phpinfo_as_text = 0;
	}

	// Return true
	RETURN_TRUE;
}
/* }}} */

/* {{{ arginfo */

/* {{{ set_exit_handler */
ZEND_BEGIN_ARG_INFO(arginfo_dt_set_exit_handler, 0)
	ZEND_ARG_INFO(0, "callback")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ dt_set_exit_exception */
ZEND_BEGIN_ARG_INFO(arginfo_dt_throw_exit_exception, 0)
	ZEND_ARG_INFO(0, "throw_exception")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ dt_set_exit_exception */
ZEND_BEGIN_ARG_INFO(arginfo_dt_set_proctitle, 0)
	ZEND_ARG_INFO(0, "title")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ dt_remove_function */
ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_function, 0)
	ZEND_ARG_INFO(0, "functionName")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ dt_remove_function */
ZEND_BEGIN_ARG_INFO(arginfo_dt_rename_function, 0)
	ZEND_ARG_INFO(0, "oldFunction")
	ZEND_ARG_INFO(0, "newFunction")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ dt_remove_function */
ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_constant, 0)
	ZEND_ARG_INFO(0, "constantName")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ dt_remove_include */
ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_include, 0)
	ZEND_ARG_INFO(0, "includeName")
ZEND_END_ARG_INFO()	
/* }}} */

/* }}} */

/* {{{ dt_remove_class */
ZEND_BEGIN_ARG_INFO(arginfo_dt_remove_class, 0)
	ZEND_ARG_INFO(0, "className")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ dt_show_plain_info */
ZEND_BEGIN_ARG_INFO(arginfo_dt_show_plain_info, 0)
	ZEND_ARG_INFO(0, "infoMode")
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ DeepTrace_zend_startup
*/
static int DeepTrace_zend_startup(zend_extension *extension)
{
	CG(compiler_options) |= ZEND_COMPILE_NO_CONSTANT_SUBSTITUTION;
	return zend_startup_module(&DeepTrace_module_entry);
}
/* }}} */

/* {{{ DeepTrace_functions[]
 *
 * Every user visible function must have an entry in DeepTrace_functions[].
 */
const zend_function_entry DeepTrace_functions[] = {
	PHP_FE(dt_set_exit_handler, arginfo_dt_set_exit_handler)
	PHP_FE(dt_throw_exit_exception, arginfo_dt_throw_exit_exception)
	PHP_FE(dt_set_proctitle, arginfo_dt_set_proctitle)
	PHP_FE(dt_remove_function, arginfo_dt_remove_function)
	PHP_FE(dt_rename_function, arginfo_dt_rename_function)
	PHP_FE(dt_remove_constant, arginfo_dt_remove_constant)
	PHP_FE(dt_remove_include, arginfo_dt_remove_include)
	PHP_FE(dt_remove_class, arginfo_dt_remove_class)
	PHP_FE(dt_show_plain_info, arginfo_dt_show_plain_info)
	PHP_FE_END	/* Must be the last line in DeepTrace_functions[] */
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(DeepTrace)
{
	// Init globals
	ZEND_INIT_MODULE_GLOBALS(DeepTrace, php_DeepTrace_init_globals, NULL);
	
	// Get argv0
	sapi_module_struct *symbol = NULL;
	symbol = &sapi_module;
	if(symbol) {
		argv0 = symbol->executable_location;
	}
	
	// Hook exit() function
	old_exit_handler = zend_get_user_opcode_handler(ZEND_EXIT);
	zend_set_user_opcode_handler(ZEND_EXIT, DeepTrace_exit_handler);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(DeepTrace)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(DeepTrace)
{
	DeepTrace_free_handler(&THG(exit_handler).fci TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
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

/* {{{ DeepTrace_module_entry
 */
zend_module_entry DeepTrace_module_entry = {
	STANDARD_MODULE_HEADER,
	"DeepTrace",
	DeepTrace_functions,
	PHP_MINIT(DeepTrace),
	PHP_MSHUTDOWN(DeepTrace),
	NULL,
	NULL,
	PHP_MINFO(DeepTrace),
	DEEPTRACE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

/* {{{ Zend extension API */
#ifndef ZEND_EXT_API
	#define ZEND_EXT_API ZEND_DLEXPORT
#endif
ZEND_EXTENSION();
/* }}} */

/* {{{ DeepTrace_extension_entry
*/
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
/* }}} */


#ifdef COMPILE_DL_DEEPTRACE
	ZEND_GET_MODULE(DeepTrace)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */