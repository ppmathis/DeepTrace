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

#include "php_DeepTrace.h"

// DeepTrace_get_zval_ptr
#if DT_PHP_VERSION == 54
	// PHP 5.4
	static zval *DeepTrace_get_zval_ptr(int op_type, znode_op *node, zval **freeval, zend_execute_data *execute_data TSRMLS_DC) /* {{{ */
	{
		*freeval = NULL;

		switch (op_type) {
			case IS_CONST: return node->zv;
			case IS_VAR: return EX_T(node->var).var.ptr;
			case IS_TMP_VAR: return (*freeval = &EX_T(node->var).tmp_var);
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
			default: return NULL;
		}
	}	
#elif DT_PHP_VERSION == 53
	// PHP 5.3
	static zval *DeepTrace_get_zval_ptr(znode *node, zval **freeval, zend_execute_data *execute_data TSRMLS_DC)
	{
		*freeval = NULL;

		switch (node->op_type) {
			case IS_CONST: return &(node->u.constant);
			case IS_VAR: return EX_T(node->u.var).var.ptr;
			case IS_TMP_VAR: return (*freeval = &EX_T(node->u.var).tmp_var);
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
			default: return NULL;
		}
	}
#endif

// DeepTrace_exit_handler
int DeepTrace_exit_handler(ZEND_OPCODE_HANDLER_ARGS)
{
	zval *exitMsg, *freeOp, *retval;

	// Check for exit handler
	if(DEEPTRACE_G(exitHandler).fci.function_name == NULL) {
		if(oldExitHandler) {
			return oldExitHandler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		} else {
			return ZEND_USER_OPCODE_DISPATCH;
		}
	}

	// Get exit message
	#if DT_PHP_VERSION == 54
		// PHP 5.4
		exitMsg = DeepTrace_get_zval_ptr(EX(opline)->op1_type, &EX(opline)->op1, &freeOp, execute_data TSRMLS_CC);
	#elif DT_PHP_VERSION == 53
		// PHP 5.3
		exitMsg = DeepTrace_get_zval_ptr(&EX(opline)->op1, &freeOp, execute_data TSRMLS_CC);
	#endif
	if(exitMsg) zend_fcall_info_argn(&DEEPTRACE_G(exitHandler).fci TSRMLS_CC, 1, &exitMsg);

	// Call user handler
	zend_fcall_info_call(&DEEPTRACE_G(exitHandler).fci, &DEEPTRACE_G(exitHandler).fcc, &retval, NULL TSRMLS_CC);
	zend_fcall_info_args_clear(&DEEPTRACE_G(exitHandler).fci, 1);
	
	// Parse return value
	convert_to_boolean(retval);
	if(Z_LVAL_P(retval)) {
		// Call default handler
		zval_ptr_dtor(&retval);
		if(oldExitHandler) {
			return oldExitHandler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		} else {
			return ZEND_USER_OPCODE_DISPATCH;
		}
	} else {
		// Call user handler
		zval_ptr_dtor(&retval);
		EX(opline)++;

		// Throw exception if desired
		if(DEEPTRACE_G(throwException)) {
			if(exitMsg && Z_TYPE_P(exitMsg) == IS_STRING) {
				zend_throw_exception(DEEPTRACE_G(exitException), Z_STRVAL_P(exitMsg), -1 TSRMLS_CC);
			} else if (exitMsg) {
				convert_to_string(exitMsg);
				zend_throw_exception(DEEPTRACE_G(exitException), Z_STRVAL_P(exitMsg), -1 TSRMLS_CC);
			} else {
				zend_throw_exception(DEEPTRACE_G(exitException), "", -1 TSRMLS_CC);
			}
			return ZEND_USER_OPCODE_CONTINUE;
		}

		return ZEND_USER_OPCODE_CONTINUE;
	}
}

// DeepTrace_set_handler
int DeepTrace_set_handler(user_opcode_handler_t opcodeHandler, int opcode, dt_opcode_handler_t *handler, INTERNAL_FUNCTION_PARAMETERS)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	// Get parameters
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f",  &fci, &fcc) == FAILURE) {
		return FAILURE;
	}

	// Is extension working?
	if(opcodeHandler != zend_get_user_opcode_handler(opcode)) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Conflicting extension detected. Make sure to load as zend extension after other extensions.");
		return FAILURE;
	}

	// Set handler
	handler->fci = fci;
	handler->fcc = fcc;
	Z_ADDREF_P(handler->fci.function_name);
	if(handler->fci.object_ptr) Z_ADDREF_P(handler->fci.object_ptr);

	return SUCCESS;
}

// DeepTrace_free_handler
void DeepTrace_free_handler(zend_fcall_info *fci)
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

// dt_set_exit_handler
// Parameters: string exitHandlerFunction
// Return value: bool success
PHP_FUNCTION(dt_set_exit_handler)
{
	if(DeepTrace_set_handler(DeepTrace_exit_handler, ZEND_EXIT, &DEEPTRACE_G(exitHandler), INTERNAL_FUNCTION_PARAM_PASSTHRU) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

// dt_throw_exit_exception
// Parameters: bool throwException
// Parameters: bool throwException, string exceptionType
// Return value: bool success
PHP_FUNCTION(dt_throw_exit_exception)
{
	int exceptionTypeLen = 0;

	// Default type name
	DEEPTRACE_G(exitExceptionType) = DEEPTRACE_EXIT_EXCEPTION_TYPE;

	// Get parameters
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b|s", &DEEPTRACE_G(throwException), &DEEPTRACE_G(exitExceptionType), &exceptionTypeLen) == FAILURE) {
		RETURN_FALSE;
	}

	// Create exception class
	INIT_OVERLOADED_CLASS_ENTRY_EX(DEEPTRACE_G(exitExceptionClass), DEEPTRACE_G(exitExceptionType), strlen(DEEPTRACE_G(exitExceptionType)), NULL, NULL, NULL, NULL, NULL, NULL);
	DEEPTRACE_G(exitException) = zend_register_internal_class_ex(&DEEPTRACE_G(exitExceptionClass), zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
}