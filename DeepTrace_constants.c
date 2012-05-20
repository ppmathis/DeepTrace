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

// dt_remove_constant
// Parameters: string constantName
// Return value: bool success
PHP_FUNCTION(dt_remove_constant)
{
	zend_constant *constant;
	char *constName, *lcase;
	int len;
	int caseSensitive;

	// Get parameters
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &constName, &len) == FAILURE) {
		RETURN_FALSE;
	}

	// Get constant
	if(zend_hash_find(EG(zend_constants), constName, len + 1, (void**) &constant) == FAILURE) {
		lcase = zend_str_tolower_dup(constName, len);
		if(zend_hash_find(EG(zend_constants), lcase, len + 1, (void**) &constant) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Constant %s not found.", constName);
			efree(lcase);
			RETURN_FALSE;
		}

		if((constant->flags & CONST_CS)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Constant %s is case-sensitive.", constant->name);
			efree(lcase);
			RETURN_FALSE;
		}
		efree(lcase);
	}

	// Safety warning for persistent constants
	if(constant->flags & CONST_PERSISTENT) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Constant %s is persistent. You should not modify it.", constant->name);
	}

	// Is case-Sensitive?
	caseSensitive = ((constant->flags & CONST_CS) == 0);
	if(caseSensitive) {
		constName = zend_str_tolower_dup(constant->name, constant->name_len);
	} else {
		constName = constant->name;
	}

	// Delete constant
	if(zend_hash_del(EG(zend_constants), constName, constant->name_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not remove constant %s.", constName);
		if(caseSensitive) efree(constName);
		RETURN_FALSE;
	}

	// Remove constant from cache
	if(DEEPTRACE_G(constantCache)) {
		zend_hash_del(DEEPTRACE_G(constantCache), constName, constant->name_len);
	}

	// Free memory
	if(caseSensitive) efree(constName);
	RETURN_TRUE;
}

// dt_get_cache_size
// Get Zend RTC size
PHP_FUNCTION(dt_get_cache_size) {
	RETVAL_LONG(EG(active_op_array)->last_cache_slot);
	return;
}

// zend_quick_get_constant
// Get constant from literal
zend_constant *zend_quick_get_constant(const zend_literal *key, ulong flags TSRMLS_DC)
{
	zend_constant *c;

	if (zend_hash_quick_find(EG(zend_constants), Z_STRVAL(key->constant), Z_STRLEN(key->constant) + 1, key->hash_value, (void **) &c) == FAILURE) {
		key++;
		if (zend_hash_quick_find(EG(zend_constants), Z_STRVAL(key->constant), Z_STRLEN(key->constant) + 1, key->hash_value, (void **) &c) == FAILURE ||
		    (c->flags & CONST_CS) != 0) {
			if ((flags & (IS_CONSTANT_IN_NAMESPACE|IS_CONSTANT_UNQUALIFIED)) == (IS_CONSTANT_IN_NAMESPACE|IS_CONSTANT_UNQUALIFIED)) {
				key++;
				if (zend_hash_quick_find(EG(zend_constants), Z_STRVAL(key->constant), Z_STRLEN(key->constant) + 1, key->hash_value, (void **) &c) == FAILURE) {
				    key++;
					if (zend_hash_quick_find(EG(zend_constants), Z_STRVAL(key->constant), Z_STRLEN(key->constant) + 1, key->hash_value, (void **) &c) == FAILURE ||
					    (c->flags & CONST_CS) != 0) {

						key--;
						if (!zend_get_special_constant(Z_STRVAL(key->constant), Z_STRLEN(key->constant), &c TSRMLS_CC)) {
							return NULL;
						}
					}
				}
			} else {
				key--;
				if (!zend_get_special_constant(Z_STRVAL(key->constant), Z_STRLEN(key->constant), &c TSRMLS_CC)) {
					return NULL;
				}
			}
		}
	}
	return c;
}

// DeepTrace_constant_handler
// Overrides intern RunTimeCache
int DeepTrace_constant_handler(ZEND_OPCODE_HANDLER_ARGS) {
	char* constName;
	int constLen;
	void* cachePtr;
	zend_constant *c;

	// Get name and length
	constName = Z_STRVAL(EX(opline)->op2.literal->constant);
	constLen = Z_STRLEN(EX(opline)->op2.literal->constant) + 1;

	// Check if cache exists
	if(!DEEPTRACE_G(constantCache)) {
		ALLOC_HASHTABLE(DEEPTRACE_G(constantCache));
		zend_hash_init(DEEPTRACE_G(constantCache), 4, NULL, NULL, 0);
	}

	// Get cache pointer
	if(zend_hash_find(DEEPTRACE_G(constantCache), constName, constLen, (void**) &cachePtr)) {
		// Detect pointer
		c = zend_quick_get_constant(EX(opline)->op2.literal + 1, EX(opline)->extended_value TSRMLS_CC);
		cachePtr = (void*) c;

		// Add to cache
		zend_printf("[DT Cache - Add] %s @ %d\n", constName, cachePtr);
		zend_hash_add(DEEPTRACE_G(constantCache), constName, constLen, (void**) &cachePtr, sizeof(void*), NULL);
	} else {
		cachePtr = *((void**) cachePtr);
		zend_printf("[DT Cache - Get] %s @ %d\n", constName, cachePtr);
	}

	// Modify Zend cache
	EG(active_op_array)->run_time_cache[EX(opline)->op2.literal->cache_slot] = cachePtr;
	return ZEND_USER_OPCODE_DISPATCH;
}