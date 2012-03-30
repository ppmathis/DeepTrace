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

// DeepTrace_fetch_function
int DeepTrace_fetch_function(char *funcName, int funcLen, zend_function **funcPtr, int flag TSRMLS_DC)
{
	zend_function *func;

	// Find function in hashtable
	if(zend_hash_find(EG(function_table), funcName, funcLen + 1, (void**) &func)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Function %s() does not exist.", funcName);
		return FAILURE;
	}

	// Is user or internal function?
	if(func->type != ZEND_USER_FUNCTION && func->type != ZEND_INTERNAL_FUNCTION) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s() is not a user or normal internal function.", funcName);
		return FAILURE;
	}

	// Set pointer
	if(funcPtr) *funcPtr = func;

	// Store modifications on internal functions
	if(func->type == ZEND_INTERNAL_FUNCTION && flag >= DEEPTRACE_FUNCTION_REMOVE) {
		// Allocate hashmap if necessary
		if(!DEEPTRACE_G(replaced_internal_functions)) {
			ALLOC_HASHTABLE(DEEPTRACE_G(replaced_internal_functions));
			zend_hash_init(DEEPTRACE_G(replaced_internal_functions), 4, NULL, NULL, 0);
		}
		zend_hash_add(DEEPTRACE_G(replaced_internal_functions), funcName, funcLen + 1, (void*) func, sizeof(zend_function), NULL);

		if(flag >= DEEPTRACE_FUNCTION_RENAME) {
			zend_hash_key hash_key;

			// Allocate hashmap if necessary
			if(!DEEPTRACE_G(misplaced_internal_functions)) {
				ALLOC_HASHTABLE(DEEPTRACE_G(misplaced_internal_functions));
				zend_hash_init(DEEPTRACE_G(misplaced_internal_functions), 4, NULL, NULL, 0);
			}
			hash_key.nKeyLength = funcLen + 1;
			hash_key.arKey = estrndup(funcName, hash_key.nKeyLength);
			zend_hash_next_index_insert(DEEPTRACE_G(misplaced_internal_functions), (void*) &hash_key, sizeof(zend_hash_key), NULL);
		}
	}
}

// DeepTrace_destroy_misplaced_functions
static int DeepTrace_destroy_misplaced_functions(zend_hash_key *hash_key TSRMLS_DC)
{
	if(!hash_key->nKeyLength) {
		return ZEND_HASH_APPLY_REMOVE;
	}

	zend_hash_del(EG(function_table), hash_key->arKey, hash_key->nKeyLength);
	return ZEND_HASH_APPLY_REMOVE;
}

// DeepTrace_restore_internal_functions
static int DeepTrace_restore_internal_functions(zend_internal_function *func TSRMLS_DC, int numArgs, va_list args, zend_hash_key *hash_key) {
	if(!hash_key->nKeyLength) {
		return ZEND_HASH_APPLY_REMOVE;
	}

	zend_hash_update(EG(function_table), hash_key->arKey, hash_key->nKeyLength, (void*) func, sizeof(zend_function), NULL);
	return ZEND_HASH_APPLY_REMOVE;
}

// dt_remove_function
// Parameters: string functionName
// Return value: bool success
PHP_FUNCTION(dt_remove_function)
{
	char *functionName;
	int len;

	// Get parameters
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &functionName, &len) == FAILURE) {
		RETURN_FALSE;
	} 

	// Make function name lowercase
	functionName = zend_str_tolower_dup(functionName, len);

	// Check if function exists
	if(!zend_hash_exists(EG(function_table), functionName, len + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Function %s does not exist.", functionName);
		efree(functionName);
		RETURN_FALSE;
	}

	// Do DeepTrace internal stuff
	if(DeepTrace_fetch_function(functionName, len, NULL, DEEPTRACE_FUNCTION_REMOVE TSRMLS_CC) == FAILURE) {
		efree(functionName);
		RETURN_FALSE;
	}

	// Delete function from table
	zend_hash_del(EG(function_table), functionName, len + 1);

	// Free memory
	efree(functionName);
	RETURN_TRUE;
}

// dt_rename_function
// Parameters: string oldFunctionName, string newFunctionName
// Return value: bool success
PHP_FUNCTION(dt_rename_function)
{
	zend_function *oldFunc, func;
	char *oldFunctionName, *newFunctionName;
	int oldLen, newLen;

	// Get parameters
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &oldFunctionName, &oldLen, &newFunctionName, &newLen) == FAILURE) {
		RETURN_FALSE;
	}

	// Make function names lowercase
	oldFunctionName = zend_str_tolower_dup(oldFunctionName, oldLen);
	newFunctionName = zend_str_tolower_dup(newFunctionName, newLen);

	// Check if new function name is free
	if(zend_hash_exists(EG(function_table), newFunctionName, newLen + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "New function name %s() already exists.", newFunctionName);
		efree(oldFunctionName);
		efree(newFunctionName);
		RETURN_FALSE;
	}

	// Check if old function name exists
	if(DeepTrace_fetch_function(oldFunctionName, oldLen, &oldFunc, DEEPTRACE_FUNCTION_RENAME TSRMLS_CC) == FAILURE) {
		efree(oldFunctionName);
		efree(newFunctionName);
		RETURN_FALSE;
	}

	// Create new function (from reference)
	func = (zend_function) *oldFunc;
	function_add_ref(&func);

	// Remove old function reference
	if(zend_hash_del(EG(function_table), oldFunctionName, oldLen + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove old reference to function %s().", oldFunctionName);
		zend_function_dtor(&func);
		efree(oldFunctionName);
		efree(newFunctionName);
		RETURN_FALSE;
	}

	// Is user function?
	if(func.type == ZEND_USER_FUNCTION) {
		efree((void *) func.common.function_name);
		func.common.function_name = estrndup(newFunctionName, newLen);
	}

	// Add new function reference
	if(zend_hash_add(EG(function_table), newFunctionName, newLen + 1, &func, sizeof(zend_function), NULL) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not create new reference to function %s().", newFunctionName);
		zend_function_dtor(&func);
		efree(oldFunctionName);
		efree(newFunctionName);
		RETURN_FALSE;
	}

	// Free memory
	efree(oldFunctionName);
	efree(newFunctionName);
	RETURN_TRUE;
}