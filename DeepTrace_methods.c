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

#ifdef DEEPTRACE_METHOD_MANIPULATION
/* {{{ dt_get_method_prototype
	Locates the prototype method */
zend_function* dt_get_method_prototype(zend_class_entry *ce, char* func, int func_len TSRMLS_DC) {
	zend_class_entry *pce = ce;
	zend_function *proto = NULL;
	char *func_lower;

	/* Make function name lowercase */
	func_lower = zend_str_tolower_dup(func, func_len);
	if (func_lower == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not enough memory available");
		return NULL;
	}

	/* Create prototype */
	while (pce) {
		if (zend_hash_find(&pce->function_table, func_lower, func_len+1, (void**) &proto) != FAILURE) {
			break;
		}
		pce = pce->parent;
	}
	if (!pce) {
		proto = NULL;
	}
	efree(func_lower);
	return proto;
}
/* }}} */

/* {{{ dt_generate_lambda_method
	Generate a lambda method */
int dt_generate_lambda_method(char *arguments, int argumentsLen, char *phpcode, int phpcodeLen, zend_function **pfe TSRMLS_DC)
{
	char *evalCode, *evalName;
	int evalCodeLen;

	evalCodeLen = sizeof("function " DT_TEMP_FUNCNAME) + argumentsLen + 4 + phpcodeLen;
	evalCode = (char*) emalloc(evalCodeLen);
	snprintf(evalCode, evalCodeLen, "function " DT_TEMP_FUNCNAME "(%s){%s}", arguments, phpcode);
	evalName = zend_make_compiled_string_description("deeptrace runtime-created function" TSRMLS_CC);
	if(zend_eval_string(evalCode, NULL, evalName TSRMLS_CC) == FAILURE) {
		efree(evalCode);
		efree(evalName);
		return FAILURE;
	}
	efree(evalCode);
	efree(evalName);

	if(zend_hash_find(EG(function_table), DT_TEMP_FUNCNAME, sizeof(DT_TEMP_FUNCNAME), (void **) pfe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unexpected inconsistency during create_function");
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ dt_fetch_class_int
	Fetch a class */
int dt_fetch_class_int(char* className, int classLen, zend_class_entry **pce TSRMLS_DC)
{
	char *lowerClass;
	zend_class_entry *ce;
	zend_class_entry **ze;

	/* Ignore leading backslash */
	if(className[0] == '\\') {
		++className;
		--classLen;
	}

	/* Make class name lowercase */
	lowerClass = zend_str_tolower_dup(className, classLen);
	if(lowerClass == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not enough memory available");
		return FAILURE;
	}

	/* Fetch class */
	if(zend_hash_find(EG(class_table), lowerClass, classLen + 1, (void**) &ze) == FAILURE || !ze || !*ze) {
		efree(lowerClass);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class %s not found", className);
		return FAILURE;
	}
	ce = *ze;

	/* Store pointer */
	if(pce) {
		*pce = ce;
	}
	efree(lowerClass);
	return SUCCESS;
}
/* }}} */

/* {{{ dt_fetch_class
	Fetch a class */
int dt_fetch_class(char* className, int classLen, zend_class_entry **pce TSRMLS_DC)
{
	zend_class_entry *ce;

	/* Fetch class */
	if(dt_fetch_class_int(className, classLen, &ce TSRMLS_CC) == FAILURE) {
		return FAILURE;
	}

	/* Warning if user tries to modify a system class */
	if(ce->type != ZEND_USER_CLASS) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Class %s is not a user-defined class", className);
	}

	/* Check for interfaces */
	if(ce->ce_flags & ZEND_ACC_INTERFACE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class %s is an interface", className);
		return FAILURE;
	}

	/* Store pointer */
	if(pce) {
		*pce = ce;
	}
	return SUCCESS;
}

/* }}} */

/* {{{ dt_fetch_class_method
	Fetch a class method */
int dt_fetch_class_method(char* className, int classLen, char* methodName, int methodLen, zend_class_entry **pce, zend_function **pfe TSRMLS_DC)
{
	HashTable *funcTable = EG(function_table);
	zend_class_entry *ce;
	zend_function *fe;
	char *lowerMethod;
	zend_class_entry **ze;

	/* Fetch class */
	if(dt_fetch_class_int(className, classLen, &ce TSRMLS_CC) == FAILURE) {
		return FAILURE;
	}

	/* Warning if user tries to modify a system class */
	if(ce->type != ZEND_USER_CLASS) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Class %s is not a user-defined class", className);
	}

	/* Store pointer */
	if(pce) {
		*pce = ce;
	}
	funcTable = &ce->function_table;

	/* Make method name lowercase */
	lowerMethod = zend_str_tolower_dup(methodName, methodLen);
	if(lowerMethod == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not enough memory available");
		return FAILURE;
	}
	
	/* Fetch method */
	if(zend_hash_find(funcTable, lowerMethod, methodLen + 1, (void**) &fe) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s::%s() not found", className, methodName);
		efree(lowerMethod);
		return FAILURE;
	}

	/* Warning if user tries to modify a system method */
	if(fe->type != ZEND_USER_FUNCTION) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Method %s::%s() is not a user-defined method", className, methodName);
	}

	/* Store pointer */
	if(pfe) {
		*pfe = fe;
	}

	efree(lowerMethod);
	return SUCCESS;
}
/* }}} */

/* {{{ dt_update_children_methods
	Update children methods */
int dt_update_children_methods(zend_class_entry *ce TSRMLS_DC, int numArgs, va_list args, zend_hash_key* hashKey)
{
	zend_class_entry *ancClass = va_arg(args, zend_class_entry*);
	zend_class_entry *parentClass = va_arg(args, zend_class_entry*);
	zend_class_entry *scope;
	zend_function *fe = va_arg(args, zend_function*);
	char *fname = va_arg(args, char*);
	int fnameLen = va_arg(args, int);
	zend_function *cfe = NULL;
	char *fnameLower;

	/* Make method name lowercase */
	fnameLower = zend_str_tolower_dup(fname, fnameLen);
	if(fnameLower == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not enough memory available");
		return FAILURE;
	}
	ce = *((zend_class_entry**) ce);

	/* Not a child... Ignore it :) */
	if(ce->parent != parentClass) {
		efree(fnameLower);
		return ZEND_HASH_APPLY_KEEP;
	}

	/* Ignore methods below our level */
	if(zend_hash_find(&ce->function_table, fnameLower, fnameLen + 1, (void**) &cfe) == SUCCESS) {
		scope = fe->common.scope;
		if(scope != ancClass) {
			efree(fnameLower);
			return ZEND_HASH_APPLY_KEEP;
		}
	}

	/* Update child class (step 1) */
	if(cfe && zend_hash_del(&ce->function_table, fnameLower, fnameLen + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error updating child class");
		efree(fnameLower);
		return ZEND_HASH_APPLY_KEEP;
	}

	/* Update child class (step 2) */
	if(zend_hash_add(&ce->function_table, fnameLower, fnameLen + 1, fe, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error updating child class");
		efree(fnameLower);
		return ZEND_HASH_APPLY_KEEP;
	}

	/* Support magic methods */
	function_add_ref(fe);
	DT_ADD_MAGIC_METHOD(ce, fname, fe);
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) dt_update_children_methods, 5, ancClass, ce, fe, fname, fnameLen);

	efree(fnameLower);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ dt_clean_children_methods
	Clean children methods */
int dt_clean_children_methods(zend_class_entry *ce TSRMLS_DC, int numArgs, va_list args, zend_hash_key *hashKey)
{
	zend_class_entry *ancClass = va_arg(args, zend_class_entry*);
	zend_class_entry *parentClass = va_arg(args, zend_class_entry*);
	zend_class_entry *scope;
	char *fname = va_arg(args, char*);
	int fnameLen = va_arg(args, int);
	zend_function *cfe = NULL;
	char *fnameLower;

	/* Make method name lowercase */
	fnameLower = zend_str_tolower_dup(fname, fnameLen);
	if(fnameLower == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not enough memory available");
		return FAILURE;
	}
	ce = *((zend_class_entry**) ce);

	/* Not a child... Ignore it :) */
	if(ce->parent != parentClass) {
		efree(fnameLower);
		return ZEND_HASH_APPLY_KEEP;
	}

	/* Ignore methods below our level */
	if(zend_hash_find(&ce->function_table, fnameLower, fnameLen + 1, (void**) &cfe) == SUCCESS) {
		scope = cfe->common.scope;
		if(scope != ancClass) {
			efree(fnameLower);
			return ZEND_HASH_APPLY_KEEP;
		}
	}

	/* Nothing to destroy... */
	if(!cfe) {
		efree(fnameLower);
		return ZEND_HASH_APPLY_KEEP;
	}

	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) dt_clean_children_methods, 4, ancClass, ce, fname, fnameLen);
	zend_hash_del(&ce->function_table, fnameLower, fnameLen + 1);
	DT_DEL_MAGIC_METHOD(ce, cfe);

	efree(fnameLower);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ proto bool dt_add_method(string className, string methodName, string args, string code)
	Add a class method */
PHP_FUNCTION(dt_add_method)
{
	char *className, *methodName, *arguments, *phpcode;
	int classLen, methodLen, argumentsLen, phpcodeLen;
	zend_class_entry *ce, *ancClass = NULL;
	zend_function func, *fe;
	char *lowerMethod;
	long argc = ZEND_NUM_ARGS();
	long flags = ZEND_ACC_PUBLIC;

	/* Get parameters */
	if(zend_parse_parameters(argc TSRMLS_CC, "ssss|l", &className, &classLen, &methodName, &methodLen, &arguments, &argumentsLen, &phpcode, &phpcodeLen, &flags) == FAILURE) {
		RETURN_FALSE;
	}

	/* Check parameters */
	if(!classLen || !methodLen) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty parameter given.");
		RETURN_FALSE;
	}

	/* Make method name lowercase */
	lowerMethod = zend_str_tolower_dup(methodName, methodLen);
	if(lowerMethod == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not enough memory available");
		RETURN_FALSE;
	}

	/* Fetch class */
	if(dt_fetch_class(className, classLen, &ce TSRMLS_CC) == FAILURE) {
		efree(lowerMethod);
		RETURN_FALSE;
	}
	ancClass = ce;

	/* Generate lambda function */
	if(dt_generate_lambda_method(arguments, argumentsLen, phpcode, phpcodeLen, &fe TSRMLS_CC) == FAILURE) {
		efree(lowerMethod);
		RETURN_FALSE;
	}

	/* Get function pointer and set flags */
	func = *fe;
	function_add_ref(&func);
	efree((void*) func.common.function_name);
	func.common.function_name = estrndup(methodName, methodLen);
	func.common.scope = ce;
	func.common.prototype = dt_get_method_prototype(ce, methodName, methodLen TSRMLS_CC);

	if(flags & ZEND_ACC_PRIVATE) {
		func.common.fn_flags &= ~ZEND_ACC_PPP_MASK;
		func.common.fn_flags |= ZEND_ACC_PRIVATE;
	} else if (flags & ZEND_ACC_PROTECTED) {
		func.common.fn_flags &= ~ZEND_ACC_PPP_MASK;
		func.common.fn_flags |= ZEND_ACC_PROTECTED;
	} else {
		func.common.fn_flags &= ~ZEND_ACC_PPP_MASK;
		func.common.fn_flags |= ZEND_ACC_PUBLIC;
	}

	if(flags & ZEND_ACC_STATIC) {
		func.common.fn_flags |= ZEND_ACC_STATIC;
	} else {
		func.common.fn_flags |= ZEND_ACC_ALLOW_STATIC;
	}

	/* Add method to hash tables */
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) dt_update_children_methods, 5, ancClass, ce, &func, methodName, methodLen);
	if(zend_hash_add(&ce->function_table, lowerMethod, methodLen + 1, &func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to add method to class");
		efree(lowerMethod);
		RETURN_FALSE;
	}

	/* Remove temporary function */
	if(zend_hash_del(EG(function_table), DT_TEMP_FUNCNAME, sizeof(DT_TEMP_FUNCNAME)) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to remove temporary function entry");
		efree(lowerMethod);
		RETURN_FALSE;
	}

	/* Locate new method */
	if(zend_hash_find(&ce->function_table, lowerMethod, methodLen + 1, (void**) &fe) == FAILURE || !fe) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to locate newly added method");
		efree(lowerMethod);
		RETURN_FALSE;
	}

	/* Support magic methods and clean up */
	DT_ADD_MAGIC_METHOD(ce, methodName, fe);
	efree(lowerMethod);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_rename_method(string className, string oldName, string newName)
	Rename a class method */
PHP_FUNCTION(dt_rename_method)
{
	char *className, *methodName, *newName;
	int classLen, methodLen, newLen;
	zend_class_entry *ce, *ancClass = NULL;
	zend_function *fe, func;
	char *newNameLower;

	/* Get parameters */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &className, &classLen, &methodName, &methodLen, &newName, &newLen) == FAILURE) {
		RETURN_FALSE;
	}

	/* Check parameters */
	if(!classLen || !methodLen || !newLen) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty parameter given.");
		RETURN_FALSE;
	}

	/* Fetch class */
	if(dt_fetch_class_method(className, classLen, methodName, methodLen, &ce, &fe TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown class method %s::%s()", className, methodName);
		RETURN_FALSE;
	}

	/* Make method name lowercase */
	newNameLower = zend_str_tolower_dup(newName, newLen);
	if(newNameLower == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not enough memory available");
		RETURN_FALSE;
	}
	methodName = zend_str_tolower_dup(methodName, methodLen);

	/* Is that method name available? */
	if(zend_hash_exists(&ce->function_table, newNameLower, newLen + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s::%s() already exists", className, newName);
		efree(newNameLower);
		RETURN_FALSE;
	}

	/* Get scope and function pointer */
	ancClass = fe->common.scope;
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) dt_clean_children_methods, 4, ancClass, ce, methodName, methodLen);
	func = *fe;
	function_add_ref(&func);
	if(fe->type == ZEND_USER_FUNCTION) {
		efree((void*) func.common.function_name);
		func.common.function_name = estrndup(newName, newLen + 1);
	}

	/* Add new method */
	if(zend_hash_add(&ce->function_table, newNameLower, newLen + 1, &func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to add new reference to class method");
		zend_function_dtor(&func);
		efree(newNameLower);
		RETURN_FALSE;
	}

	/* Remove old method */
	if(zend_hash_del(&ce->function_table, methodName, methodLen + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to remove old method reference from class");
		efree(newNameLower);
		RETURN_FALSE;
	}
	DT_DEL_MAGIC_METHOD(ce, fe);

	/* Try if the method exists now */
	if(dt_fetch_class_method(className, classLen, newName, newLen, &ce, &fe TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to locate the newly renamed method");
		efree(newNameLower);
		RETURN_FALSE;
	}

	/* Clean up and support magic methods */
	efree(newNameLower);
	efree(methodName);
	DT_ADD_MAGIC_METHOD(ce, newName, fe);
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) dt_update_children_methods, 5, ce, ce, fe, newName, newLen);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_remove_method(string className, string methodName)
	Remove a method from a class */
PHP_FUNCTION(dt_remove_method)
{
	char *className, *methodName;
	int classLen, methodLen;
	zend_class_entry *ce, *ancClass = NULL;
	zend_function *fe;
	char *lowerMethod;

	/* Get parameters */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &className, &classLen, &methodName, &methodLen) == FAILURE) {
		RETURN_FALSE;
	}

	/* Check parameters */
	if(!classLen || !methodLen) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty parameter given.");
		RETURN_FALSE;
	}

	/* Fetch class */
	if(dt_fetch_class_method(className, classLen, methodName, methodLen, &ce, &fe TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown class method %s::%s()", className, methodName);
		RETURN_FALSE;
	}

	/* Make method name lowercase */
	lowerMethod = zend_str_tolower_dup(methodName, methodLen);
	if(lowerMethod == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not enough memory available");
		RETURN_FALSE;
	}
	ancClass = fe->common.scope;

	/* Update children and delete method */
	zend_hash_apply_with_arguments(EG(class_table) TSRMLS_CC, (apply_func_args_t) dt_clean_children_methods, 4, ancClass, ce, methodName, methodLen);
	if(zend_hash_del(&ce->function_table, lowerMethod, methodLen + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to remove method from class");
		efree(lowerMethod);
		RETURN_FALSE;
	}

	/* Support magic methods and clean up */
	efree(lowerMethod);
	DT_DEL_MAGIC_METHOD(ce, fe);
	RETURN_TRUE;
}
/* }}} */
#endif