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

/* {{{ DeepTrace_remove_class
	Remove a class / trait / interface */
int DeepTrace_remove_class(char *name, int len, int type)
{
	zend_class_entry removedClass;
	char *lcase;

	/* Make class name lower case */
	lcase = zend_str_tolower_dup(name, len);

	/* Check if class exists */
	if(!zend_hash_exists(EG(class_table), lcase, len + 1)) {
		if(type == DEEPTRACE_REMOVE_CLASS) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class %s does not exist.", name);
		if(type == DEEPTRACE_REMOVE_INTERFACE) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Interface %s does not exist.", name);
		if(type == DEEPTRACE_REMOVE_TRAIT) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Trait %s does not exist.", name);
		efree(lcase);
		return 0;
	}
	
	/* Remove class in hash map */
	if(zend_hash_del(EG(class_table), lcase, len + 1) == FAILURE) {
		if(type == DEEPTRACE_REMOVE_CLASS) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove class %s.", name);
		if(type == DEEPTRACE_REMOVE_INTERFACE) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove interface %s.", name);
		if(type == DEEPTRACE_REMOVE_TRAIT) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove trait %s.", name);
		efree(lcase);
		return 0;
	}

	/* Free memory */
	efree(lcase);
	return 1;
}
/* }}} */

/* {{{ proto bool dt_remove_class(string className)
	Remove a class with the given name */
PHP_FUNCTION(dt_remove_class)
{
	char *className;
	int len;

	/* Get class name */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &className, &len) == FAILURE) {
		RETURN_FALSE;
	}
	
	/* Return value */
	if(DeepTrace_remove_class(className, len, DEEPTRACE_REMOVE_CLASS)) RETURN_TRUE;
	RETURN_FALSE;
}

/* {{{ proto bool dt_remove_interface(string interfaceName)
	Remove an interface with the given name */
PHP_FUNCTION(dt_remove_interface)
{
	char *interfaceName;
	int len;

	/* Get class name */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &interfaceName, &len) == FAILURE) {
		RETURN_FALSE;
	}

	/* Return value */
	if(DeepTrace_remove_class(interfaceName, len, DEEPTRACE_REMOVE_INTERFACE)) RETURN_TRUE;
	RETURN_FALSE;
}

/* {{{ proto bool dt_remove_trait(string traitName)
	Remove a trait with the given name */
#if DT_PHP_VERSION == 54
	/* PHP 5.4 */
	PHP_FUNCTION(dt_remove_trait)
	{
		char *traitName;
		int len;

		/* Get class name */
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &traitName, &len) == FAILURE) {
			RETURN_FALSE;
		}
		
		/* Return value */
		if(DeepTrace_remove_class(traitName, len, DEEPTRACE_REMOVE_TRAIT)) RETURN_TRUE;
		RETURN_FALSE;
	}
#elif DT_PHP_VERSION == 53
	/* PHP 5.3 */
	PHP_FUNCTION(dt_remove_trait)
	{
		char *traitName;
		int len;

		/* Get class name */
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &traitName, &len) == FAILURE) {
			RETURN_FALSE;
		}
		
		/* Return value */
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Traits are not supported in PHP 5.3. Tried to remove trait %s.", traitName);
		RETURN_FALSE;
	}
#endif

#endif