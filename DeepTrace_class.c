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

/* {{{ PHP_FUNCTION(dt_remove_class) */
PHP_FUNCTION(dt_remove_class)
{
	DEEPTRACE_DECL_STRING_PARAM(className);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(className)) == FAILURE) {
		RETURN_FALSE;
	}

	/* Make class name lower case */
	className = zend_str_tolower_dup(className, className_len);

	/* Remove class in hash table */
	if(zend_hash_del(EG(class_table), className, className_len + 1) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove class, trait or interface '%s'.", className);
		efree(className);
		RETURN_FALSE;
	}

	efree(className);
	RETURN_TRUE;
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_destroy_class_data) */
PHP_FUNCTION(dt_destroy_class_data)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	zend_class_entry **class;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(className)) == FAILURE) {
		RETURN_FALSE;
	}

	/* Make class name lowercase */
	className = zend_str_tolower_dup(className, className_len);
	if(zend_hash_find(EG(class_table), className, className_len + 1, (void **) &class) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class '%s' not found.", className);
		efree(className);
		RETURN_FALSE;
	}

	zend_cleanup_class_data(class TSRMLS_CC);
	efree(className);
	RETURN_TRUE;
}
/* }}} */

/* {{{ DeepTrace_destroy_class_constants */
static int DeepTrace_destroy_class_constants(char *name TSRMLS_DC, int numArgs, va_list args, zend_hash_key *hashKey)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	DEEPTRACE_DECL_STRING_PARAM(cacheName);

	/* Get arguments */
	className = va_arg(args, char *);
	className_len = va_arg(args, int);

	/* Create combined name */
	cacheName = emalloc(className_len + hashKey->nKeyLength + 2);
	cacheName_len = className_len + hashKey->nKeyLength + 1;
	sprintf(cacheName, "%s::%s", className, hashKey->arKey);

	/* Delete constant from constant cache */
#ifdef DEEPTRACE_DEBUG_CONSTANT_CACHE
	printf("[DT Constant Cache] Del> %s\n", cacheName);
#endif
	zend_hash_del(DEEPTRACE_G(constantCache), cacheName, cacheName_len + 1);
	efree(cacheName);

	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_destroy_class_consts) */
PHP_FUNCTION(dt_destroy_class_consts)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	zend_class_entry **class;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(className)) == FAILURE) {
		RETURN_FALSE;
	}

	/* Make class name lowercase */
	className = zend_str_tolower_dup(className, className_len);
	if(zend_hash_find(EG(class_table), className, className_len + 1, (void **) &class) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class '%s' not found.", className);
		efree(className);
		RETURN_FALSE;
	}

	zend_hash_apply_with_arguments(&((zend_class_entry) **class).constants_table,
			(apply_func_args_t) DeepTrace_destroy_class_constants, 2, className, className_len);
	efree(className);
}
/* }}} */
