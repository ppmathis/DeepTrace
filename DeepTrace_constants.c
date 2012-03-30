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
	if((constant->flags & CONST_CS) == 0) {
		constName = zend_str_tolower_dup(constName, len);
	} else {
		constName = constant->name;
	}

	// Delete constant
	if(zend_hash_del(EG(zend_constants), constName, len + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not remove constant %s.", constName);
		if((constant->flags & CONST_CS) == 0) efree(constName);
		RETURN_FALSE;
	}

	// Free memory
	if((constant->flags & CONST_CS) == 0) efree(constName);
	RETURN_TRUE;
}