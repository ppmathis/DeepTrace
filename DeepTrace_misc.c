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
static zend_bool setproctitle(char *title, int title_len TSRMLS_DC)
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

/* {{{ proto bool dt_set_proctitle(string title)
   Sets the title of the current process */
PHP_FUNCTION(dt_set_proctitle)
{
	DEEPTRACE_DECL_STRING_PARAM(title);

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", DEEPTRACE_STRING_PARAM(title)) == FAILURE)) {
		RETURN_FALSE;
	}

#ifndef DEEPTRACE_SYSTEM_PROVIDES_SETPROCTITLE
	/* Local setproctitle function if there is no native call */
	if(EXPECTED(setproctitle(title, title_len TSRMLS_CC) == SUCCESS)) RETURN_TRUE;
	RETURN_FALSE;
#else
	/* Use the native call when it is available */
	setproctitle("%s", title);
	RETURN_TRUE;
#endif
}
/* }}} */

/* {{{ proto bool dt_phpinfo_mode(bool phpInfoAsText)
   Switches between text-only and HTML phpinfo() mode */
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

/* {{{ proto bool dt_remove_include(string includeName)
   Removes a file from the list of included files */
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

/* {{{ proto dt_inspect_zval(mixed value)
   Dumps refcount, type and memory address of a zval */
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

#if ZEND_DEBUG
/* Copy of PHP function */
static int DeepTrace_object_property_dump(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key) /* {{{ */
{
	int level;
	const char *prop_name, *class_name;

	level = va_arg(args, int);

	if (hash_key->nKeyLength == 0) { /* numeric key */
		php_printf("%*c[%ld]=>\n", level + 1, ' ', hash_key->h);
	} else { /* string key */
		int unmangle = zend_unmangle_property_name(hash_key->arKey, hash_key->nKeyLength - 1, &class_name, &prop_name);
		php_printf("%*c[", level + 1, ' ');

		if (class_name && unmangle == SUCCESS) {
			if (class_name[0] == '*') {
				php_printf("\"%s\":protected", prop_name);
			} else {
				php_printf("\"%s\":\"%s\":private", prop_name, class_name);
			}
		} else {
			php_printf("\"");
			PHPWRITE(hash_key->arKey, hash_key->nKeyLength - 1);
			php_printf("\"");
		}
		ZEND_PUTS("]=>\n");
	}
	php_var_dump(zv, level + 2 TSRMLS_CC);
	return 0;
}
/* }}} */

/* {{{ proto dt_debug_objects_store([int dumpObjects = 0])
   Lists all entries in the PHP objects store. Available only when PHP was compiled with --enable-debug */
PHP_FUNCTION(dt_debug_objects_store)
{
	zend_objects_store store = EG(objects_store);
	int i;
	struct _store_object *obj;
	zend_object *zobj;
	HashTable *freeTable;
	int dumpObjects = 0;

	ALLOC_HASHTABLE(freeTable);
	zend_hash_init_ex(freeTable, 0, NULL, NULL, 1, 0);

	if(UNEXPECTED(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &dumpObjects) == FAILURE)) {
		RETURN_FALSE;
	}

	printf("---- DEBUG ZEND OBJECTS STORE ----\n");

	/* Dump basic object store data */
	printf("Top: %u\n", store.top);
	printf("Size: %u\n", store.size);
	printf("Free List Head: %d\n", store.free_list_head);

	/* Check for free buckets in object store */
	for(i = 0;i < store.top;i++) {
		if(!zend_hash_index_exists(freeTable, EG(objects_store).object_buckets[i].bucket.free_list.next)) {
			zend_hash_index_update(freeTable, EG(objects_store).object_buckets[i].bucket.free_list.next, NULL, 0, NULL);
		}
	}

	for(i = 0;i < store.top;i++) {
		/* Ignore bucket if free */
		if(i == store.free_list_head || zend_hash_index_exists(freeTable, i)) {
			continue;
		}

		obj = &EG(objects_store).object_buckets[i].bucket.obj;
		zobj = obj->object;

		/* Ignore object if NULL */
		if(obj == NULL || zobj == NULL) {
			continue;
		}

		/* Rebuild property table if necessary */
		if(!zobj->properties) {
			rebuild_object_properties(zobj);
		}

		/* Output object ID, class and refcount */
		printf("Object %d of class %s at %#x: %u references, %d properties\n", i, zobj->ce->name, zobj, obj->refcount, zend_hash_num_elements(zobj->properties));

		if(dumpObjects) {
			/* Dump properties */
			zend_hash_apply_with_arguments(zobj->properties TSRMLS_CC, (apply_func_args_t) DeepTrace_object_property_dump, 1, 1);
		}
	}

	/* Free memory */
	zend_hash_destroy(freeTable);
	efree(freeTable);

	printf("---- END DEBUG ZEND OBJECTS STORE ----\n");
}
/* }}} */
#endif
