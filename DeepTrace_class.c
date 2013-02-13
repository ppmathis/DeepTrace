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

/* {{{ proto bool dt_remove_class(string className)
   Deletes a PHP class */
PHP_FUNCTION(dt_remove_class)
{
	DEEPTRACE_DECL_STRING_PARAM(className);

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(className)) == FAILURE)) {
		RETURN_FALSE;
	}

	/* Make class name lower case */
	className = zend_str_tolower_dup(className, className_len);

	/* Remove class in hash table */
	if(UNEXPECTED(zend_hash_del(EG(class_table), className, className_len + 1) == FAILURE)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove class, trait or interface '%s'.", className);
		efree(className);
		RETURN_FALSE;
	}

	DeepTrace_clear_all_functions_runtime_cache(TSRMLS_C);
	efree(className);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool dt_destroy_class_data(string className)
   Destroys the data of a PHP class. */
PHP_FUNCTION(dt_destroy_class_data)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	zend_class_entry **class;

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(className)) == FAILURE)) {
		RETURN_FALSE;
	}

	/* Make class name lowercase */
	className = zend_str_tolower_dup(className, className_len);
	if(UNEXPECTED(zend_hash_find(EG(class_table), className, className_len + 1, (void **) &class) == FAILURE)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Class '%s' not found.", className);
		efree(className);
		RETURN_FALSE;
	}

	zend_cleanup_class_data(class TSRMLS_CC);
	efree(className);
	RETURN_TRUE;
}
/* }}} */
