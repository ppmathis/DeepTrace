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

#ifdef DEEPTRACE_THREAD_SUPPORT
/* {{{ proto bool dt_set_proctitle(string processTitle)
	Set the process title of the current process / thread */
PHP_FUNCTION(dt_set_proctitle)
{
	char *procTitle;
	int len;

	/* Get parameters */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &procTitle, &len) == FAILURE) {
		RETURN_FALSE;
	}

	/* Set process title */
	#ifdef DEEPTRACE_SYSTEM_HAS_SETPROCTITLE
		/* The direct way (recommended) */
		setproctitle("%s", procTitle);
	#else
		/* Declare buffer */
		char buffer[DEEPTRACE_PROCTITLE_MAX_LEN];

		/* Check for argv0 */
		if(!DEEPTRACE_G(argv0)) RETURN_FALSE;

		/* The argv0 workaround */
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
/* }}} */
#endif

#ifdef DEEPTRACE_INFO_MANIPULATION
/* {{{ proto bool dt_show_plain_info(bool textOnly)
	Set the phpinfo output mode */
PHP_FUNCTION(dt_show_plain_info)
{
	/* Get parameters */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &DEEPTRACE_G(infoMode)) == FAILURE) {
		RETURN_FALSE;
	}

	/* Set info mode */
	sapi_module.phpinfo_as_text = DEEPTRACE_G(infoMode);

	RETURN_TRUE;
}
/* }}} */
#endif

#ifdef DEEPTRACE_INCLUDE_MANIPULATION
/* {{{ proto bool dt_remove_include(string includeName)
	Remove a include in Zend */
PHP_FUNCTION(dt_remove_include)
{
	char *includeName, *absolutePath;
	int len;

	/* Get include name */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &includeName, &len) == FAILURE) {
		RETURN_FALSE;
	}

	/* Find include in hash map */
	absolutePath = zend_resolve_path(includeName, len);

	if(!absolutePath) {
		absolutePath = estrdup(includeName);
	}

	if(!zend_hash_exists(&EG(included_files), absolutePath, strlen(absolutePath) + 1)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Include %s does not exist.", includeName);
		efree(absolutePath);
		RETURN_FALSE;
	}	

	/* Remove include from hash map */
	if(zend_hash_del(&EG(included_files), absolutePath, strlen(absolutePath) + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove include %s.", includeName);
		efree(absolutePath);
		RETURN_FALSE;
	}

	/* Free memory */
	efree(absolutePath);
	RETURN_TRUE;
}
/* }}} */
#endif
