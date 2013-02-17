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

#include "php_DeepTrace.h"

/* {{{ DeepTrace_get_zval_ptr */
static zval *DeepTrace_get_zval_ptr(int op_type, znode_op *node, zval **freeval,
		zend_execute_data *execute_data, zend_free_op *should_free TSRMLS_DC)
{
	*freeval = NULL;
	should_free->var = 0;

	switch(op_type) {
	case IS_CONST:
		return node->zv;
	case IS_VAR:
		if(Z_DELREF_P(EX_T(node->var).var.ptr) == 0) {
			should_free->var = EX_T(node->var).var.ptr;
		}
		return EX_T(node->var).var.ptr;
	case IS_TMP_VAR:
#if DEEPTRACE_PHP_VERSION >= 55
		return (*freeval = &EX_TMP_VAR_NUM(execute_data, node->var)->tmp_var);
#else
		return (*freeval = &EX_T(node->var).tmp_var);
#endif
	case IS_CV:
	{
#if DEEPTRACE_PHP_VERSION >= 55
		zval ***ret = EX_CV_NUM(execute_data, node->var);
#else
		zval ***ret = &execute_data->CVs[node->var];
#endif
		if(!*ret) {
			zend_compiled_variable *cv = &EG(active_op_array)->vars[node->var];
			if(zend_hash_quick_find(EG(active_symbol_table), cv->name, cv->name_len + 1,
					cv->hash_value, (void**) ret) == FAILURE) {
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
/* }}} */

/* {{{ DeepTrace_exit_cleanup */
void DeepTrace_exit_cleanup(TSRMLS_D)
{
	/* Clean up old exit handler if there is one */
	zend_fcall_info *fci = &DEEPTRACE_G(exitHandler).fci;
	if(fci->function_name) {
		zval_ptr_dtor(&fci->function_name);
		fci->function_name = NULL;
	}
	if(fci->object_ptr) {
		zval_ptr_dtor(&fci->object_ptr);
		fci->object_ptr = NULL;
	}

	/* Destroy old exception class */
	if(DEEPTRACE_G(exitException)) {
		/* Cleanup class data */
		DEEPTRACE_G(exitException)->parent = zend_exception_get_default(TSRMLS_C);
		zend_cleanup_class_data(&DEEPTRACE_G(exitException) TSRMLS_CC);

		/* Remove old exception class from class table */
		char *className = zend_str_tolower_dup(DEEPTRACE_G(exitException)->name,
				DEEPTRACE_G(exitException)->name_length);
		if(zend_hash_del(EG(class_table), className, strlen(className) + 1) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Can not remove old exception class.");
		}
		efree(className);

		DEEPTRACE_G(exitException) = NULL;
	}

	DEEPTRACE_G(exitMode) = DEEPTRACE_EXIT_NORMAL;
}
/* }}} */

/* {{{ DeepTrace_exit_handler */
int DeepTrace_exit_handler(ZEND_OPCODE_HANDLER_ARGS)
{
	zval *exitMsg, *tmp, *retval;
	zend_free_op should_free;

	/* If there is no user handler specified, call the default one */
	if(DEEPTRACE_G(exitMode) == DEEPTRACE_EXIT_NORMAL) {
		if(DEEPTRACE_G(exitOldHandler)) {
			return DEEPTRACE_G(exitOldHandler)(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		} else {
			return ZEND_USER_OPCODE_DISPATCH;
		}
	}

	/* Get exit message if there was one */
	exitMsg = DeepTrace_get_zval_ptr(EX(opline)->op1_type, &EX(opline)->op1, &tmp, execute_data, &should_free TSRMLS_CC);
	if(EXPECTED(exitMsg != NULL)) {
		Z_ADDREF_P(exitMsg);
		zend_fcall_info_argn(&DEEPTRACE_G(exitHandler).fci TSRMLS_CC, 1, &exitMsg);
	}

	/* Call user handler */
	zend_fcall_info_call(&DEEPTRACE_G(exitHandler).fci, &DEEPTRACE_G(exitHandler).fcc, &retval, NULL TSRMLS_CC);
	zend_fcall_info_args_clear(&DEEPTRACE_G(exitHandler).fci, 1);

	convert_to_boolean(retval);
	if(Z_LVAL_P(retval)) {
		/* Call default handler if the return value was true */
		zval_ptr_dtor(&retval);

		if(DEEPTRACE_G(exitOldHandler)) {
			return DEEPTRACE_G(exitOldHandler)(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		} else {
			return ZEND_USER_OPCODE_DISPATCH;
		}
	} else {
		/* Continue with execution or throw an exception if
		 * the return value was false
		 */
		zval_ptr_dtor(&retval);
		EX(opline)++;

		/* Throw exception if the current exit mode requires it */
		if(DEEPTRACE_G(exitMode) == DEEPTRACE_EXIT_EXCEPTION) {
			DEEPTRACE_G(exitException)->parent = zend_exception_get_default(TSRMLS_C);

			if(EXPECTED(exitMsg != NULL)) {
				zval *message, *exception;

				ALLOC_ZVAL(message);
				INIT_PZVAL_COPY(message, exitMsg);
				zval_copy_ctor(message);

				if(Z_TYPE_P(exitMsg) != IS_STRING) {
					convert_to_string(message);
				}

				exception = zend_throw_exception(DEEPTRACE_G(exitException), NULL, -1 TSRMLS_CC);
				zend_update_property(DEEPTRACE_G(exitException), exception, "message", sizeof("message") - 1, message TSRMLS_CC);
				zval_ptr_dtor(&message);
			} else {
				zend_throw_exception(DEEPTRACE_G(exitException), "", -1 TSRMLS_CC);
			}

			DEEPTRACE_G(exitException)->parent = NULL;
		}

		if(should_free.var) {
			zval_ptr_dtor(&should_free.var);
		}

		if(tmp != NULL) {
			zval_dtor(tmp);
		}

		return ZEND_USER_OPCODE_CONTINUE;
	}
}
/* }}} */

/* {{{ DeepTrace_exit_set_handler */
static int DeepTrace_exit_set_handler(user_opcode_handler_t handler, deeptrace_opcode_handler_t userHandler TSRMLS_DC)
{
	/* Check if custom opcode hook was successful during module initialization */
	if(UNEXPECTED(handler != zend_get_user_opcode_handler(ZEND_EXIT))) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Conflicting extension detected. "
				"Make sure to load DeepTrace as a zend extension.");
		return FAILURE;
	}

	/* Set the new handler */
	DeepTrace_exit_cleanup(TSRMLS_C);
	DEEPTRACE_G(exitHandler).fci = userHandler.fci;
	DEEPTRACE_G(exitHandler).fcc = userHandler.fcc;
	Z_ADDREF_P(DEEPTRACE_G(exitHandler).fci.function_name);
	if(DEEPTRACE_G(exitHandler).fcc.object_ptr) Z_ADDREF_P(DEEPTRACE_G(exitHandler).fcc.object_ptr);

	return SUCCESS;
}
/* }}} */

/* {{{ proto bool dt_exit_mode(int exitMode [, callable handlerFunction[, string exceptionName]])
   Sets or resets the exit handling mode */
PHP_FUNCTION(dt_exit_mode) {
	long exitMode;
	DEEPTRACE_DECL_HANDLER_PARAM(handlerFunction);
	DEEPTRACE_DECL_STRING_PARAM(exceptionName);

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|fs", &exitMode,
			DEEPTRACE_HANDLER_PARAM(handlerFunction),
			DEEPTRACE_STRING_PARAM(exceptionName)) == FAILURE)) {
		RETURN_FALSE;
	}

	switch(ZEND_NUM_ARGS()) {
	case 1:
		/* DEEPTRACE_EXIT_NORMAL */
		if(EXPECTED(exitMode == DEEPTRACE_EXIT_NORMAL)) {
			DeepTrace_exit_cleanup(TSRMLS_C);
			DEEPTRACE_G(exitMode) = DEEPTRACE_EXIT_NORMAL;

			RETURN_TRUE;
		}
		break;
	case 2:
		/* DEEPTRACE_EXIT_HANDLER */
		if(EXPECTED(exitMode == DEEPTRACE_EXIT_HANDLER)) {
			DeepTrace_exit_cleanup(TSRMLS_C);

			/* Set handler */
			if(UNEXPECTED(DeepTrace_exit_set_handler(DeepTrace_exit_handler, handlerFunction TSRMLS_CC) != SUCCESS))
				RETURN_FALSE;

			/* Create instance of exception class */
			DEEPTRACE_G(exitMode) = DEEPTRACE_EXIT_HANDLER;
			RETURN_TRUE;
		}

		break;
	case 3:
		/* DEEPTRACE_EXIT_EXCEPTION */
		if(EXPECTED(exitMode == DEEPTRACE_EXIT_EXCEPTION)) {
			DeepTrace_exit_cleanup(TSRMLS_C);

			/* Set handler */
			if(UNEXPECTED(DeepTrace_exit_set_handler(DeepTrace_exit_handler, handlerFunction TSRMLS_CC) != SUCCESS))
				RETURN_FALSE;

			/* Create exception class */
			zend_class_entry class;
			INIT_OVERLOADED_CLASS_ENTRY_EX(class, exceptionName, exceptionName_len,
			 		NULL, NULL, NULL, NULL, NULL, NULL);
			DEEPTRACE_G(exitException) = zend_register_internal_class_ex(&class,
					zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);

			/* Set current mode and return */
			DEEPTRACE_G(exitMode) = DEEPTRACE_EXIT_EXCEPTION;
			RETURN_TRUE;
		}

		break;
	}

	/* Invalid parameter count */
	php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"dt_exit_mode() got an invalid parameter count, %lu given", ZEND_NUM_ARGS());
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto dt_exit_fetch_exception()
   Makes an exit exception an instance of the Exception class. */
PHP_FUNCTION(dt_exit_fetch_exception) {
	if(DEEPTRACE_G(exitMode) != DEEPTRACE_EXIT_EXCEPTION) return;
	if(!DEEPTRACE_G(exitException) || !DEEPTRACE_G(exitException)->parent) return;

	DEEPTRACE_G(exitException)->parent = zend_exception_get_default(TSRMLS_C);
}
/* }}} */
