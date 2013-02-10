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

/* {{{ DeepTrace_get_method_prototype */
static zend_function *DeepTrace_get_method_prototype(zend_class_entry *ce, char *funcName, int funcName_len TSRMLS_DC)
{
	zend_class_entry *pce = ce;
	zend_function *proto = NULL;
	ulong hash;

	/* Convert function name to lowercase and get hash */
	funcName = zend_str_tolower_dup(funcName, funcName_len);
	hash = zend_inline_hash_func(funcName, funcName_len + 1);

	/* Find method prototype */
	while(pce) {
		if(zend_hash_quick_find(&pce->function_table, funcName, funcName_len + 1, hash, (void **) &proto) != FAILURE) {
			break;
		}
		pce = pce->parent;
	}

	if(!pce) proto = NULL;
	efree(funcName);
	return proto;
}
/* }}} */

/* {{{ DeepTrace_generate_lambda_Method */
static int DeepTrace_generate_lambda_method(char *arguments, int arguments_len, char *phpcode, int phpcode_len,
		zend_function **pfe TSRMLS_DC)
{
	DEEPTRACE_DECL_STRING_PARAM(evalCode);
	char *evalName;

	evalCode_len = sizeof("function " DEEPTRACE_TEMP_FUNCNAME) + 4 + arguments_len + phpcode_len;
	evalCode = (char *) emalloc(evalCode_len);
	snprintf(evalCode, evalCode_len, "function " DEEPTRACE_TEMP_FUNCNAME "(%s){%s}", arguments, phpcode);

	evalName = zend_make_compiled_string_description("DeepTrace RTC function" TSRMLS_CC);
	if(zend_eval_stringl(evalCode, evalCode_len - 1, NULL, evalName TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot create temporary function '%s'.", DEEPTRACE_TEMP_FUNCNAME);
		efree(evalCode);
		efree(evalName);
		return FAILURE;
	}

	efree(evalCode);
	efree(evalName);

	if(zend_hash_find(EG(function_table), DEEPTRACE_TEMP_FUNCNAME, sizeof(DEEPTRACE_TEMP_FUNCNAME),
			(void *) pfe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unexpected inconsistency while trying to create lambda method.");
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ DeepTrace_fetch_class_int */
static int DeepTrace_fetch_class_int(char *className, int className_len, zend_class_entry **pce TSRMLS_DC)
{
	zend_class_entry *ce, **ze;
	char *lowerClassName;

	/* Ignore leading backslash */
	if(className[0] == '\\') {
		++className;
		--className_len;
	}
	lowerClassName = zend_str_tolower_dup(className, className_len);

	/* Fetch class from hash table */
	if(zend_hash_find(EG(class_table), lowerClassName, className_len + 1, (void **) &ze) == FAILURE || !ze || !*ze) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class '%s' not found.", lowerClassName);
		efree(lowerClassName);
		return FAILURE;
	}

	ce = *ze;
	if(pce) *pce = ce;
	efree(lowerClassName);
	return SUCCESS;
}
/* }}} */

/* {{{ DeepTrace_fetch_class */
static int DeepTrace_fetch_class(char *className, int className_len, zend_class_entry **pce TSRMLS_DC)
{
	zend_class_entry *ce;

	/* Fetch class */
	if(DeepTrace_fetch_class_int(className, className_len, &ce TSRMLS_CC) == FAILURE) {
		return FAILURE;
	}

	/* Warning if user tries to modify a internal class */
	if(ce->type != ZEND_USER_CLASS) {
#ifdef DEEPTRACE_NOTICES
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Class '%s' is not a user-defined class.", className);
#endif
	}

	/* Check if it is a interface */
	if(ce->ce_flags & ZEND_ACC_INTERFACE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class '%s' is an interface.", className);
		return FAILURE;
	}

	if(pce) *pce = ce;
	return SUCCESS;
}
/* }}} */

/* {{{ DeepTrace_fetch_class_method */
static int DeepTrace_fetch_class_method(char *className, int className_len, char *methodName, int methodName_len,
		zend_class_entry **pce, zend_function **pfe TSRMLS_DC)
{
	zend_class_entry *ce, **ze;
	zend_function *fe;

	/* Fetch class */
	if(DeepTrace_fetch_class_int(className, className_len, &ce TSRMLS_CC) == FAILURE) {
		return FAILURE;
	}

	/* Warning if user tries to modify a internal class */
	if(ce->type != ZEND_USER_CLASS) {
#ifdef DEEPTRACE_NOTICES
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Class '%s' is not a user-defined class.", className);
#endif
	}
	if(pce) *pce = ce;

	/* Convert method name to lowercase and fetch method */
	methodName = zend_str_tolower_dup(methodName, methodName_len);
	if(zend_hash_find(&ce->function_table, methodName, methodName_len + 1, (void **) &fe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Method %s::%s() not found", className, methodName);
		efree(methodName);
		return FAILURE;
	}

	/* Warning if user tries to modify a internal method */
	if(fe->type != ZEND_USER_FUNCTION) {
#ifdef DEEPTRACE_NOTICES
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Method '%s::%s()' is not a user-defined method.",
				className, methodName);
#endif
	}
	if(pfe) *pfe = fe;

	efree(methodName);
	return SUCCESS;
}
/* }}} */

/* {{{ DeepTrace_update_children_methods */
static int DeepTrace_update_children_methods(zend_class_entry *ce TSRMLS_DC, int numArgs,
		va_list args, zend_hash_key *hashKey)
{
	zend_class_entry *ancestorClass = va_arg(args, zend_class_entry *);
	zend_class_entry *parentClass = va_arg(args, zend_class_entry *);
	zend_class_entry *scope;
	zend_function *fe = va_arg(args, zend_function *);
	zend_function *cfe = NULL;
	char *funcName = va_arg(args, char *);
	int funcName_len = va_arg(args, int);
	zend_function *original_fe = va_arg(args, zend_function *);
	ulong hash;

	/* If it is not a child, it can be ignored */
	ce = *((zend_class_entry **) ce);
	if(ce->parent != parentClass) {
		return ZEND_HASH_APPLY_KEEP;
	}

	/* Convert method name to lowercase and get hash */
	funcName = zend_str_tolower_dup(funcName, funcName_len);
	hash = zend_inline_hash_func(funcName, funcName_len + 1);

	/* Ignore methods below our level */
	if(zend_hash_quick_find(&ce->function_table, funcName, funcName_len + 1, hash, (void **) &cfe) == SUCCESS) {
		scope = fe->common.scope;
		if(scope != ancestorClass) {
			cfe->common.prototype = DeepTrace_get_method_prototype(scope->parent, funcName, funcName_len TSRMLS_CC);
			zend_hash_apply_with_arguments(EG(class_table), (apply_func_args_t) DeepTrace_update_children_methods,
					6, ancestorClass, ce, fe, funcName, funcName_len, original_fe);
			efree(funcName);
			return ZEND_HASH_APPLY_KEEP;
		}
	}

	/* Update child class */
	if(cfe && zend_hash_quick_del(&ce->function_table, funcName, funcName_len + 1, hash) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unexpected inconsistency while updating child class.");
		efree(funcName);
		return ZEND_HASH_APPLY_KEEP;
	}

	if(zend_hash_quick_add(&ce->function_table, funcName, funcName_len + 1, hash,
			fe, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unexpected inconsistency while updating child class.");
		efree(funcName);
		return ZEND_HASH_APPLY_KEEP;
	}

	/* Support magic methods */
	function_add_ref(fe);
	DEEPTRACE_INHERIT_MAGIC_METHOD(ce, fe, original_fe);
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) DeepTrace_update_children_methods,
			6, ancestorClass, ce, fe, funcName, funcName_len, original_fe);

	efree(funcName);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ DeepTrace_clean_children_methods */
static int DeepTrace_clean_children_methods(zend_class_entry *ce TSRMLS_DC, int numArgs,
		va_list args, zend_hash_key *hashKey)
{
	zend_class_entry *ancestorClass = va_arg(args, zend_class_entry *);
	zend_class_entry *parentClass = va_arg(args, zend_class_entry *);
	zend_class_entry *scope;
	zend_function *cfe = NULL;
	char *funcName = va_arg(args, char *);
	int funcName_len = va_arg(args, int);
	zend_function *original_cfe = va_arg(args, zend_function *);
	ulong hash;

	/* If it is not a child, it can be ignored */
	ce = *((zend_class_entry **) ce);
	if(ce->parent != parentClass) {
		return ZEND_HASH_APPLY_KEEP;
	}

	/* Convert method name to lowercase and get hash */
	funcName = zend_str_tolower_dup(funcName, funcName_len);
	hash = zend_inline_hash_func(funcName, funcName_len + 1);

	/* Ignore methods below our level */
	if(zend_hash_quick_find(&ce->function_table, funcName, funcName_len + 1, hash, (void **) &cfe) == SUCCESS) {
		scope = cfe->common.scope;
		if(scope != ancestorClass) {
			efree(funcName);
			return ZEND_HASH_APPLY_KEEP;
		}
	}

	/* Nothing to destroy? */
	if(!cfe) {
		efree(funcName);
		return ZEND_HASH_APPLY_KEEP;
	}

	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) DeepTrace_clean_children_methods,
			5, ancestorClass, ce, funcName, funcName_len, original_cfe);
	zend_hash_quick_del(&ce->function_table, funcName, funcName_len + 1, hash);
	DEEPTRACE_DEL_MAGIC_METHOD(ce, original_cfe);

	efree(funcName);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ DeepTrace_misplaced_methods_destructor */
static void DeepTrace_misplaced_methods_destructor(void *pElement)
{
	zend_hash_key *hashKey = (zend_hash_key *) pElement;
	efree(hashKey->arKey);
}
/* }}} */

/* {{{ DeepTrace_misplaced_methods */
static void DeepTrace_misplaced_methods(int type, char *className, int className_len,
		char *methodName, int methodName_len)
{
	/* Initialize hash table if not yet initialized */
	if(!DEEPTRACE_G(misplaced_internal_methods)) {
		ALLOC_HASHTABLE(DEEPTRACE_G(misplaced_internal_methods));
		zend_hash_init(DEEPTRACE_G(misplaced_internal_methods), 4, NULL, DeepTrace_misplaced_methods_destructor, 0);
	}

	/* Create combined name */
	char *combinedName;
	int combinedName_len;
	combinedName = emalloc(className_len + methodName_len + 2);
	sprintf(combinedName, "%s%c%s", className, '\1', methodName);
	combinedName_len = strlen(combinedName);

	switch(type) {
	case DEEPTRACE_MISPLACED_METHOD_ADD:
	{
		zend_hash_key hashKey;
		hashKey.arKey = combinedName;
		hashKey.nKeyLength = combinedName_len + 1;
		zend_hash_add(DEEPTRACE_G(misplaced_internal_methods), combinedName, combinedName_len + 1,
				(void *) &hashKey, sizeof(zend_hash_key), NULL);
		break;
	}
	case DEEPTRACE_MISPLACED_METHOD_REMOVE:
	{
		zend_hash_del(DEEPTRACE_G(misplaced_internal_methods), combinedName, combinedName_len + 1);
		efree(combinedName);
		break;
	}
	}
}
/* }}} */

/* {{{ DeepTrace_remove_method */
static int DeepTrace_remove_method(char *className, int className_len, char *methodName, int methodName_len,
		zend_bool dontTouchMisplacedMethods)
{
	zend_class_entry *ce, *ancestorClass = NULL;
	zend_function *fe;

	/* Fetch class method */
	if(DeepTrace_fetch_class_method(className, className_len, methodName, methodName_len,
			&ce, &fe TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown class method '%s::%s()'.", className, methodName);
		return FAILURE;
	}

	/* Convert method name to lowercase */
	methodName = zend_str_tolower_dup(methodName, methodName_len);
	ancestorClass = fe->common.scope;

	/* Update childrens and delete method */
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) DeepTrace_clean_children_methods,
			5, ancestorClass, ce, methodName, methodName_len, fe);
	DeepTrace_clear_all_functions_runtime_cache();
	if(zend_hash_del(&ce->function_table, methodName, methodName_len + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to remove method '%s::%s()'.", className, methodName);
		efree(methodName);
		return FAILURE;
	}

	/* Remove method in misplaced methods hash table */
	if(ce->type != ZEND_USER_CLASS && !dontTouchMisplacedMethods) {
		DeepTrace_misplaced_methods(DEEPTRACE_MISPLACED_METHOD_REMOVE, className, className_len,
				methodName, methodName_len);
	}

	efree(methodName);
	DEEPTRACE_DEL_MAGIC_METHOD(ce, fe);
	return SUCCESS;
}
/* }}} */

/* {{{ DeepTrace_destroy_misplaced_methods */
static int DeepTrace_destroy_misplaced_methods(zend_hash_key *hashKey TSRMLS_DC)
{
	if(!hashKey->nKeyLength) return ZEND_HASH_APPLY_REMOVE;

	char *className, *methodName;
	className = strtok((char *) hashKey->arKey, "\1");
	methodName = strtok(NULL, "\1");
	DeepTrace_remove_method(className, strlen(className), methodName, strlen(methodName), 1);

	return ZEND_HASH_APPLY_REMOVE;
}
/* }}} */

/* {{{ DeepTrace_methods_cleanup */
void DeepTrace_methods_cleanup()
{
	if(DEEPTRACE_G(misplaced_internal_methods)) {
		zend_hash_apply(DEEPTRACE_G(misplaced_internal_methods),
						(apply_func_t) DeepTrace_destroy_misplaced_methods TSRMLS_CC);
		zend_hash_destroy(DEEPTRACE_G(misplaced_internal_methods));
		FREE_HASHTABLE(DEEPTRACE_G(misplaced_internal_methods));
	}
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_add_method) */
PHP_FUNCTION(dt_add_method)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	DEEPTRACE_DECL_STRING_PARAM(methodName);
	DEEPTRACE_DECL_STRING_PARAM(arguments);
	DEEPTRACE_DECL_STRING_PARAM(phpcode);
	zend_class_entry *ce, *ancestorClass = NULL;
	zend_function func, *fe;
	long flags = ZEND_ACC_PUBLIC;
	ulong hash;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssss|l",
			DEEPTRACE_STRING_PARAM(className),
			DEEPTRACE_STRING_PARAM(methodName),
			DEEPTRACE_STRING_PARAM(arguments),
			DEEPTRACE_STRING_PARAM(phpcode),
			&flags) == FAILURE) {
		RETURN_FALSE;
	}

	/* Check parameters */
	if(!className_len || !methodName_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty parameters given.");
		RETURN_FALSE;
	}

	/* Convert method name to lowercase and get hash */
	methodName = zend_str_tolower_dup(methodName, methodName_len);
	hash = zend_inline_hash_func(methodName, methodName_len + 1);

	/* Fetch class */
	if(DeepTrace_fetch_class(className, className_len, &ce TSRMLS_CC) == FAILURE) {
		efree(methodName);
		RETURN_FALSE;
	}
	ancestorClass = ce;

	/* Check if new method name is free */
	if(zend_hash_quick_exists(&ce->function_table, methodName, methodName_len + 1, hash)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Method '%s::%s()' already exists.", className, methodName);
		efree(methodName);
		RETURN_FALSE;
	}

	/* Generate lambda method */
	if(DeepTrace_generate_lambda_method(arguments, arguments_len, phpcode, phpcode_len, &fe TSRMLS_CC) == FAILURE) {
		efree(methodName);
		RETURN_FALSE;
	}

	/* Get function from pointer and set flags */
	func = *fe;
	function_add_ref(&func);
	efree((void *) func.common.function_name);
	func.common.function_name = estrndup(methodName, methodName_len);
	func.common.scope = ce;
	func.common.prototype = DeepTrace_get_method_prototype(ce, methodName, methodName_len TSRMLS_CC);
	func.common.fn_flags &= ~ZEND_ACC_PPP_MASK;

	if(flags & ZEND_ACC_PRIVATE) {
		func.common.fn_flags |= ZEND_ACC_PRIVATE;
	} else if (flags & ZEND_ACC_PROTECTED) {
		func.common.fn_flags |= ZEND_ACC_PROTECTED;
	} else {
		func.common.fn_flags |= ZEND_ACC_PUBLIC;
	}

	if(flags & ZEND_ACC_STATIC) {
		func.common.fn_flags |= ZEND_ACC_STATIC;
	} else {
		func.common.fn_flags |= ZEND_ACC_ALLOW_STATIC;
	}

	/* Add method to hash table */
	DeepTrace_clear_all_functions_runtime_cache();
	if(zend_hash_quick_add(&ce->function_table, methodName, methodName_len + 1, hash,
			&func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to add method '%s::%s()'", className, methodName);
		efree(methodName);
		RETURN_FALSE;
	}

	/* Remove temporary function from hash table */
	if(zend_hash_del(EG(function_table), DEEPTRACE_TEMP_FUNCNAME, sizeof(DEEPTRACE_TEMP_FUNCNAME)) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unable to remove temporary function '%s'.",
				DEEPTRACE_TEMP_FUNCNAME);
		efree(methodName);
		RETURN_FALSE;
	}

	/* Locate new method */
	if(zend_hash_quick_find(&ce->function_table, methodName, methodName_len + 1, hash,
			(void **) &fe) == FAILURE || !fe) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unexpected consistency while locating new method '%s::%s'.",
				className, methodName);
		efree(methodName);
		RETURN_FALSE;
	}

	/* Add method in misplaced methods hash table */
	if(ce->type != ZEND_USER_CLASS) {
		DeepTrace_misplaced_methods(DEEPTRACE_MISPLACED_METHOD_ADD, className, className_len,
				methodName, methodName_len);
	}

	fe->common.scope = ce;
	fe->common.prototype = DeepTrace_get_method_prototype(ce->parent, methodName, methodName_len TSRMLS_CC);
	DEEPTRACE_ADD_MAGIC_METHOD(ce, methodName, methodName_len, fe, NULL);
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) DeepTrace_update_children_methods,
				6, ancestorClass, ce, &func, methodName, methodName_len, NULL);

	efree(methodName);
	RETURN_TRUE;
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_rename_method) */
PHP_FUNCTION(dt_rename_method)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	DEEPTRACE_DECL_STRING_PARAM(oldMethodName);
	DEEPTRACE_DECL_STRING_PARAM(newMethodName);
	zend_class_entry *ce, *ancestorClass = NULL;
	zend_function *fe, func;
	ulong oldMethodHash, newMethodHash;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss",
			DEEPTRACE_STRING_PARAM(className),
			DEEPTRACE_STRING_PARAM(oldMethodName),
			DEEPTRACE_STRING_PARAM(newMethodName)) == FAILURE) {
		RETURN_FALSE;
	}

	/* Check parameters */
	if(!className_len || !oldMethodName_len || !newMethodName_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty parameters given.");
		RETURN_FALSE;
	}

	/* Fetch class method */
	if(DeepTrace_fetch_class_method(className, className_len, oldMethodName, oldMethodName_len,
			&ce, &fe TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown class method '%s::%s()'.", className, oldMethodName);
		RETURN_FALSE;
	}

	/* Convert method names to lowercase and get hash */
	oldMethodName = zend_str_tolower_dup(oldMethodName, oldMethodName_len);
	newMethodName = zend_str_tolower_dup(newMethodName, newMethodName_len);
	oldMethodHash = zend_inline_hash_func(oldMethodName, oldMethodName_len + 1);
	newMethodHash = zend_inline_hash_func(newMethodName, newMethodName_len + 1);

	/* Check if new method name is free */
	if(zend_hash_quick_exists(&ce->function_table, newMethodName, newMethodName_len + 1, newMethodHash)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Method '%s::%s()' already exists.", className, newMethodName);
		efree(oldMethodName);
		efree(newMethodName);
		RETURN_FALSE;
	}

	/* Get anchor class and function pointer */
	ancestorClass = fe->common.scope;
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) DeepTrace_clean_children_methods,
			4, ancestorClass, ce, oldMethodName, oldMethodName_len);
	DeepTrace_clear_all_functions_runtime_cache();

	func = *fe;
	function_add_ref(&func);

	if(fe->type == ZEND_USER_FUNCTION) {
		efree((void*) func.common.function_name);
		func.common.function_name = estrndup(newMethodName, newMethodName_len);
	}

	/* Add method in hash table */
	if(zend_hash_quick_add(&ce->function_table, newMethodName, newMethodName_len + 1, newMethodHash,
			&func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to create new method '%s::%s()'.",
				className, newMethodName);
		efree(oldMethodName);
		efree(newMethodName);
		RETURN_FALSE;
	}

	/* Remove old method in hash table */
	if(zend_hash_quick_del(&ce->function_table, oldMethodName, oldMethodName_len + 1, oldMethodHash) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to remove old method '%s::%s()'.",
				className, newMethodName);
		efree(oldMethodName);
		efree(newMethodName);
		RETURN_FALSE;
	}

	DEEPTRACE_DEL_MAGIC_METHOD(ce, fe);

	/* Locate new method in hash table */
	if(DeepTrace_fetch_class_method(className, className_len, newMethodName, newMethodName_len,
			&ce, &fe TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unexpected consistency while locating new method '%s::%s'.",
				className, newMethodName);
		efree(oldMethodName);
		efree(newMethodName);
		RETURN_FALSE;
	}

	/* Rename method in misplaced methods hash table */
	if(ce->type != ZEND_USER_CLASS) {
		DeepTrace_misplaced_methods(DEEPTRACE_MISPLACED_METHOD_REMOVE, className, className_len,
				oldMethodName, oldMethodName_len);
		DeepTrace_misplaced_methods(DEEPTRACE_MISPLACED_METHOD_ADD, className, className_len,
				newMethodName, newMethodName_len);
	}

	DEEPTRACE_ADD_MAGIC_METHOD(ce, newMethodName, newMethodName_len, fe, NULL);
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) DeepTrace_update_children_methods,
			6, ce, ce, fe, newMethodName, newMethodName_len, NULL);
	efree(oldMethodName);
	efree(newMethodName);
	RETURN_TRUE;
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_remove_method) */
PHP_FUNCTION(dt_remove_method)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	DEEPTRACE_DECL_STRING_PARAM(methodName);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			DEEPTRACE_STRING_PARAM(className),
			DEEPTRACE_STRING_PARAM(methodName)) == FAILURE) {
		RETURN_FALSE;
	}

	/* Check parameters */
	if(!className_len || !methodName) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty parameters given.");
		RETURN_FALSE;
	}

	/* Call internal removal function */
	if(DeepTrace_remove_method(className, className_len, methodName, methodName_len, 0) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_set_static_method_variable) */
PHP_FUNCTION(dt_set_static_method_variable)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	DEEPTRACE_DECL_STRING_PARAM(methodName);
	DEEPTRACE_DECL_STRING_PARAM(variableName);
	zend_class_entry *ce;
	zval *value, **variablePointer;
	zend_function *func;
	zend_uchar isRef;
	int refcount;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sssz",
			DEEPTRACE_STRING_PARAM(className),
			DEEPTRACE_STRING_PARAM(methodName),
			DEEPTRACE_STRING_PARAM(variableName),
			&value) == FAILURE) {
		RETURN_FALSE;
	}

	/* Convert method name to lowercase */
	methodName = zend_str_tolower_dup(methodName, methodName_len);
	if(DeepTrace_fetch_class_method(className, className_len, methodName, methodName_len,
			&ce, &func TSRMLS_CC) == FAILURE) {
		efree(methodName);
		RETURN_FALSE;
	}

	/* Check if static variable exists */
	if(func->op_array.static_variables == NULL || zend_hash_find(func->op_array.static_variables,
			variableName, variableName_len + 1, (void **) &variablePointer) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Static variable '%s' in method '%s::%s()' does not exist.",
				className, methodName, variableName);
		efree(methodName);
		RETURN_FALSE;
	}

	/* Set static variable */
	refcount = Z_REFCOUNT_PP(variablePointer);
	isRef = Z_ISREF_PP(variablePointer);
	zval_dtor(*variablePointer);
	**variablePointer = *value;
	zval_copy_ctor(*variablePointer);
	Z_SET_REFCOUNT_PP(variablePointer, refcount);
	Z_SET_ISREF_TO_PP(variablePointer, isRef);

	efree(methodName);
	RETURN_TRUE;
}
/* }}} */
