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

/* {{{ PHP_FUNCTION(dt_remove_constant) */
PHP_FUNCTION(dt_remove_constant)
{
	DEEPTRACE_DECL_STRING_PARAM(constantName);
	char *lcase;
	zend_bool caseSensitive;
	zend_constant *constant;
	ulong hash;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(constantName)) == FAILURE) {
		RETURN_FALSE;
	}

	/* Find constant in hash table */
	if(zend_hash_find(EG(zend_constants), constantName, constantName_len + 1, (void **) &constant) == FAILURE) {
		lcase = zend_str_tolower_dup(constantName, constantName_len);

		/* Check if case-sensitive constant exists */
		if(zend_hash_find(EG(zend_constants), lcase, constantName_len + 1, (void **) &constant) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Constant '%s' not found.", constantName);
			efree(lcase);
			RETURN_FALSE;
		}

		/* Check if constant is case-sensitive */
		if(constant->flags & CONST_CS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Constant '%s' is case-sensitive.", constantName);
			efree(lcase);
			RETURN_FALSE;
		}

		efree(lcase);
	}

	/* Output safety warning for persistent constants */
	if(constant->flags & CONST_PERSISTENT) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE,
				"Constant '%s' is persistent. You can, but you should not touch it.", constantName);
	}

	/* Get constant name and hash */
	caseSensitive = (constant->flags & CONST_CS) == 0;
	if(caseSensitive) {
		constantName = zend_str_tolower_dup(constant->name, constant->name_len);
	} else {
		constantName = constant->name;
	}
	hash = zend_inline_hash_func(constantName, constant->name_len);

	/* Delete constant in hash table */
	if(zend_hash_quick_del(EG(zend_constants), constantName, constant->name_len, hash) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove constant '%s'.", constantName);
		if(caseSensitive) efree(constantName);
		RETURN_FALSE;
	}

	DeepTrace_clear_all_functions_runtime_cache(TSRMLS_C);

	if(caseSensitive) efree(constantName);
	RETURN_TRUE;
}
/* }}} */
