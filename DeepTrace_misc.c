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
	if(UNEXPECTED(!DEEPTRACE_G(argv0))) {
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

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(title)) == FAILURE)) {
		RETURN_FALSE;
	}

#ifndef DEEPTRACE_SYSTEM_PROVIDES_SETPROCTITLE
	/* Local setproctitle function if there is no native call */
	if(EXPECTED(setproctitle(title, title_len) == SUCCESS)) RETURN_TRUE;
	RETURN_FALSE;
#else
	/* Use the native call when it is available */
	if(EXPECTED(setproctitle("%s", title))) RETURN_TRUE;
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_phpinfo_mode) */
PHP_FUNCTION(dt_phpinfo_mode)
{
	zend_bool phpinfoMode;

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &phpinfoMode) == FAILURE)) {
		RETURN_FALSE;
	}

	sapi_module.phpinfo_as_text = phpinfoMode;
	RETURN_TRUE;
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_remove_include) */
PHP_FUNCTION(dt_remove_include)
{
	DEEPTRACE_DECL_STRING_PARAM(includeName);
	DEEPTRACE_DECL_STRING_PARAM(absolutePath);

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(includeName)) == FAILURE)) {
		RETURN_FALSE;
	}

	/* Resolve absolute path */
	absolutePath = zend_resolve_path(includeName, includeName_len TSRMLS_CC);
	if(!absolutePath) absolutePath = estrdup(includeName);
	absolutePath_len = strlen(absolutePath);

	/* Remove include */
	if(UNEXPECTED(zend_hash_del(&EG(included_files), absolutePath, absolutePath_len + 1) == FAILURE)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove include: %s", absolutePath);
		efree(absolutePath);
		RETURN_FALSE;
	}

	efree(absolutePath);
	RETURN_TRUE;
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_inspect_zval) */
PHP_FUNCTION(dt_inspect_zval)
{
	zval *val;
	char *addr;
	int addr_len;

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &val) == FAILURE)) {
		RETURN_FALSE;
	}

	array_init(return_value);
	addr_len = spprintf(&addr, 0, "0x%01x", (long) val);

	add_assoc_stringl(return_value, "address", addr, addr_len, 0);
	add_assoc_long(return_value, "refcount", val->refcount__gc);
	add_assoc_bool(return_value, "is_ref", val->is_ref__gc);
	add_assoc_long(return_value, "type", val->type);
}
/* }}} */
