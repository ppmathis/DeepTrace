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

/* {{{ setproctitle */
#ifndef DEEPTRACE_SYSTEM_PROVIDES_SETPROCTITLE
static zend_bool setproctitle(char *title, int title_len)
{
	char buffer[DEEPTRACE_PROCTITLE_MAX_LEN];

	/* When there is no argv0 available, we can not do anything. */
	if(!DEEPTRACE_G(argv0)) {
		return FAILURE;
	}

	/* Truncate title if longer than buffer */
	if(title_len >= (DEEPTRACE_PROCTITLE_MAX_LEN - 1)) {
		title_len = DEEPTRACE_PROCTITLE_MAX_LEN - 1;
	}

	/* Copy string into buffer and output it into argv0 */
	memset(buffer, 0x20, DEEPTRACE_PROCTITLE_MAX_LEN);
	buffer[DEEPTRACE_PROCTITLE_MAX_LEN - 1] = '\0';
	memcpy(buffer, title, title_len);
	snprintf(DEEPTRACE_G(argv0), DEEPTRACE_PROCTITLE_MAX_LEN, "%s", buffer);

	return SUCCESS;
}
#endif
/* }}} */

/* {{{ PHP_FUNCTION(dt_set_proctitle) */
PHP_FUNCTION(dt_set_proctitle)
{
	DEEPTRACE_DECL_STRING_PARAM(title);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(title)) == FAILURE) {
		RETURN_FALSE;
	}

#ifndef DEEPTRACE_SYSTEM_PROVIDES_SETPROCTITLE
	/* Local setproctitle function if there is no native call */
	if(setproctitle(title, title_len) == SUCCESS) RETURN_TRUE;
	RETURN_FALSE;
#else
	/* Use the native call when it is available */
	if(setproctitle("%s", title)) RETURN_TRUE;
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_phpinfo_mode) */
PHP_FUNCTION(dt_phpinfo_mode)
{
	zend_bool phpinfoMode;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &phpinfoMode) == FAILURE) {
		RETURN_FALSE;
	}

	sapi_module.phpinfo_as_text = phpinfoMode;
	RETURN_TRUE;
}
/* }}} */
