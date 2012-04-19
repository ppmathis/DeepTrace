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

static const zend_function_entry removedClassFuncs[] = {ZEND_FE_END};

// DeepTrace_remove_class
int DeepTrace_remove_class(char *name, int len, int type)
{
	zend_class_entry removedClass;
	char *lcase;

	// Make class name lower case
	lcase = zend_str_tolower_dup(name, len);

	// Check if class exists
	if(!zend_hash_exists(EG(class_table), lcase, len + 1)) {
		if(type == DEEPTRACE_REMOVE_CLASS) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class %s does not exist.", name);
		if(type == DEEPTRACE_REMOVE_INTERFACE) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Interface %s does not exist.", name);
		if(type == DEEPTRACE_REMOVE_TRAIT) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Trait %s does not exist.", name);
		efree(lcase);
		return 0;
	}
	
	// Remove class in hash map
	if(zend_hash_del(EG(class_table), lcase, len + 1) == FAILURE) {
		if(type == DEEPTRACE_REMOVE_CLASS) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove class %s.", name);
		if(type == DEEPTRACE_REMOVE_INTERFACE) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove interface %s.", name);
		if(type == DEEPTRACE_REMOVE_TRAIT) php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove trait %s.", name);
		efree(lcase);
		return 0;
	}

	// Free memory
	efree(lcase);
	return 1;
}

// dt_set_proctitle
// Parameters: string processTitle
// Return value: bool success
PHP_FUNCTION(dt_set_proctitle)
{
	char *procTitle;
	int len;

	// Get parameters
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &procTitle, &len) == FAILURE) {
		RETURN_FALSE;
	}

	// Set process title
	#ifdef DEEPTRACE_SYSTEM_HAS_SETPROCTITLE
		// The direct way (recommended)
		setproctitle("%s", procTitle);
	#else
		// Declare buffer
		char buffer[DEEPTRACE_PROCTITLE_MAX_LEN];

		// Check for argv0
		if(!DEEPTRACE_G(argv0)) RETURN_FALSE;

		// The argv0 workaround
		memset(buffer, 0x20, DEEPTRACE_PROCTITLE_MAX_LEN);
		if(len >= (DEEPTRACE_PROCTITLE_MAX_LEN - 1)) {
			len = DEEPTRACE_PROCTITLE_MAX_LEN - 1;
		}
		buffer[DEEPTRACE_PROCTITLE_MAX_LEN - 1] = '\0';
		memcpy(buffer, procTitle, len);
		snprintf(DEEPTRACE_G(argv0), DEEPTRACE_PROCTITLE_MAX_LEN, "%s", buffer);
	#endif

	RETURN_TRUE;
}

// dt_show_plain_info
// Parameters: bool infoMode
// Return value: bool success
PHP_FUNCTION(dt_show_plain_info)
{
	// Get parameters
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &DEEPTRACE_G(infoMode)) == FAILURE) {
		RETURN_FALSE;
	}

	// Set info mode
	sapi_module.phpinfo_as_text = DEEPTRACE_G(infoMode);

	RETURN_TRUE;
}

// dt_remove_include
// Parameters: string includeName
// Return value: bool success
PHP_FUNCTION(dt_remove_include)
{
	char *includeName, *absolutePath;
	int len;

	// Get include name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &includeName, &len) == FAILURE) {
		RETURN_FALSE;
	}

	// Find include in hash map
	absolutePath = zend_resolve_path(includeName, len);
	if(!zend_hash_exists(&EG(included_files), absolutePath, strlen(absolutePath) + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Include %s does not exist.", includeName);
		efree(absolutePath);
		RETURN_FALSE;
	}	

	// Remove include from hash map
	if(zend_hash_del(&EG(included_files), absolutePath, strlen(absolutePath) + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove include %s.", includeName);
		efree(absolutePath);
		RETURN_FALSE;
	}

	// Free memory
	efree(absolutePath);
	RETURN_TRUE;
}

// dt_remove_class
// Parameters: string className
// Return value: bool success
PHP_FUNCTION(dt_remove_class)
{
	char *className;
	int len;

	// Get class name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &className, &len) == FAILURE) {
		RETURN_FALSE;
	}
	
	// Return value
	if(DeepTrace_remove_class(className, len, DEEPTRACE_REMOVE_CLASS)) RETURN_TRUE;
	RETURN_FALSE;
}

// dt_remove_interface
// Parameters: string interfaceName
// Return value: bool success
PHP_FUNCTION(dt_remove_interface)
{
	char *interfaceName;
	int len;

	// Get class name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &interfaceName, &len) == FAILURE) {
		RETURN_FALSE;
	}

	// Return value
	if(DeepTrace_remove_class(interfaceName, len, DEEPTRACE_REMOVE_INTERFACE)) RETURN_TRUE;
	RETURN_FALSE;
}

// dt_remove_trait
// Parameters: string traitName
// Return value: bool success
#if DT_PHP_VERSION == 54
	// PHP 5.4
	PHP_FUNCTION(dt_remove_trait)
	{
		char *traitName;
		int len;

		// Get class name
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &traitName, &len) == FAILURE) {
			RETURN_FALSE;
		}
		
		// Return value
		if(DeepTrace_remove_class(traitName, len, DEEPTRACE_REMOVE_TRAIT)) RETURN_TRUE;
		RETURN_FALSE;
	}
#elif DT_PHP_VERSION == 53
	// PHP 5.3
	PHP_FUNCTION(dt_remove_trait)
	{
		char *traitName;
		int len;

		// Get class name
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &traitName, &len) == FAILURE) {
			RETURN_FALSE;
		}
		
		// Return value
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Traits are not supported in PHP 5.3. Tried to remove trait %s.", traitName);
		RETURN_FALSE;
	}
#endif