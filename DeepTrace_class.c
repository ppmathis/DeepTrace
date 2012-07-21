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

#ifdef DEEPTRACE_CLASS_MANIPULATION

static const zend_function_entry removedClassFuncs[] = {ZEND_FE_END};

/* {{{ proto bool dt_remove_class(string className)
	Remove a class, interface or trait with the given name */
PHP_FUNCTION(dt_remove_class)
{
	char *className, *lcase;
	int len;
	ulong h;

	/* Get class name */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &className, &len) == FAILURE) {
		RETURN_FALSE;
	}
	
	/* Make class name lower case */
	lcase = zend_str_tolower_dup(className, len);

	/* Get hash */
	h = zend_inline_hash_func(lcase, len + 1);

	/* Check if class exists */
	if(!zend_hash_quick_exists(EG(class_table), lcase, len + 1, h)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class, trait or interface %s does not exist.", className);

		efree(lcase);
		RETURN_FALSE;
	}

	/* Remove class in hash map */
	if(zend_hash_quick_del(EG(class_table), lcase, len + 1, h) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove class, trait or interface %s.", className);

		efree(lcase);
		RETURN_FALSE;
	}

	/* Free memory */
	efree(lcase);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_destroy_class_data(string className)
 	Destroy the data of a class, trait or interface with the given name */
PHP_FUNCTION(dt_destroy_class_data)
{
	char *className, *lcase;
	int len;
	zend_class_entry **class;

	/* Get class name */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &className, &len) == FAILURE) {
		RETURN_FALSE;
	}

	/* Make class name lower case */
	lcase = zend_str_tolower_dup(className, len);

	/* Get class entry */
	if(zend_hash_find(EG(class_table), lcase, len + 1, (void **) &class) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class %s not found.", lcase);

		efree(lcase);
		RETURN_FALSE;
	}

	/* Clean data */
	zend_cleanup_class_data(class TSRMLS_CC);

	/* Free memory */
	efree(lcase);

	RETURN_TRUE;
}
/* }}} */

#endif
