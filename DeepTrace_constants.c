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

/* {{{ PHP_FUNCTION(dt_clear_constant_cache) */
PHP_FUNCTION(dt_clear_constant_cache)
{
	if(DEEPTRACE_G(constantCache)) {
		/* Destroy old constant cache */
		zend_hash_destroy(DEEPTRACE_G(constantCache));
		FREE_HASHTABLE(DEEPTRACE_G(constantCache));
		DEEPTRACE_G(constantCache) = NULL;

		/* Allocate new constant cache */
		ALLOC_HASHTABLE(DEEPTRACE_G(constantCache));
		zend_hash_init(DEEPTRACE_G(constantCache), 4, NULL, NULL, 0);
	}
}
/* }}} */

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
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Constant '%s' is persistent. You should not touch it.");
	}

	/* Get constant name and hash */
	caseSensitive = (constant->flags & CONST_CS) == 0;
	if(caseSensitive) {
		constantName = zend_str_tolower_dup(constant->name, constant->name_len);
	} else {
		constantName = constant->name;
	}
	hash = zend_inline_hash_func(constantName, constant->name_len);

	/* Remove old entry from constant cache */
	if(DEEPTRACE_G(constantCache)) {
		zend_hash_quick_del(DEEPTRACE_G(constantCache), constantName, constant->name_len, hash);
	}

	/* Delete constant in hash table */
	if(zend_hash_quick_del(EG(zend_constants), constantName, constant->name_len, hash) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not remove constant '%s'.", constantName);
		if(caseSensitive) efree(constantName);
		RETURN_FALSE;
	}

	if(caseSensitive) efree(constantName);
	RETURN_TRUE;
}
/* }}} */

/* {{{ PHP_FUNCTION(dt_get_constant_cache_stats) */
PHP_FUNCTION(dt_get_constant_cache_stats)
{
	if(!DEEPTRACE_G(constantCache)) RETURN_FALSE;

	array_init(return_value);
	add_assoc_long(return_value, "count", DEEPTRACE_G(constantCache)->nNumOfElements);
	add_assoc_long(return_value, "zendCount", EG(active_op_array)->last_cache_slot);
}
/* }}} */

/* {{{ zend_get_special_constant */
static int zend_get_special_constant(const char *name, uint name_len, zend_constant **c TSRMLS_DC)
{
	int ret;
	static char haltoff[] = "__COMPILER_HALT_OFFSET__";

	if (!EG(in_execution)) {
		return 0;
	} else if (name_len == sizeof("__CLASS__")-1 &&
			  !memcmp(name, "__CLASS__", sizeof("__CLASS__")-1)) {
		zend_constant tmp;

		/* Returned constants may be cached, so they have to be stored */
		if (EG(scope) && EG(scope)->name) {
			int const_name_len;
			char *const_name;
			ulong h;
			ALLOCA_FLAG(use_heap)

			const_name_len = sizeof("\0__CLASS__") + EG(scope)->name_length;
			const_name = do_alloca(const_name_len, use_heap);
			memcpy(const_name, "\0__CLASS__", sizeof("\0__CLASS__")-1);
			zend_str_tolower_copy(const_name + sizeof("\0__CLASS__")-1, EG(scope)->name, EG(scope)->name_length);

			/* Get hash */
			h = zend_inline_hash_func(const_name, const_name_len);

			if (zend_hash_quick_find(EG(zend_constants), const_name, const_name_len, h, (void**)c) == FAILURE) {
				zend_hash_quick_add(EG(zend_constants), const_name, const_name_len, h, (void*)&tmp, sizeof(zend_constant), (void**)c);
				memset(*c, 0, sizeof(zend_constant));
				Z_STRVAL((**c).value) = estrndup(EG(scope)->name, EG(scope)->name_length);
				Z_STRLEN((**c).value) = EG(scope)->name_length;
				Z_TYPE((**c).value) = IS_STRING;
			}
			free_alloca(const_name, use_heap);
		} else {
			ulong h;

			/* Get hash */
			h = zend_inline_hash_func( "\0__CLASS__", sizeof("\0__CLASS__"));

			if (zend_hash_quick_find(EG(zend_constants), "\0__CLASS__", sizeof("\0__CLASS__"), h, (void**)c) == FAILURE) {
				zend_hash_quick_add(EG(zend_constants), "\0__CLASS__", sizeof("\0__CLASS__"), h, (void*)&tmp, sizeof(zend_constant), (void**)c);
				memset(*c, 0, sizeof(zend_constant));
				Z_STRVAL((**c).value) = estrndup("", 0);
				Z_STRLEN((**c).value) = 0;
				Z_TYPE((**c).value) = IS_STRING;
			}
		}
		return 1;
	} else if (name_len == sizeof("__COMPILER_HALT_OFFSET__")-1 &&
			  !memcmp(name, "__COMPILER_HALT_OFFSET__", sizeof("__COMPILER_HALT_OFFSET__")-1)) {
		const char *cfilename;
		char *haltname;
		int len, clen;

		cfilename = zend_get_executed_filename(TSRMLS_C);
		clen = strlen(cfilename);
		/* check for __COMPILER_HALT_OFFSET__ */
		zend_mangle_property_name(&haltname, &len, haltoff,
			sizeof("__COMPILER_HALT_OFFSET__") - 1, cfilename, clen, 0);
		ret = zend_hash_find(EG(zend_constants), haltname, len+1, (void **) c);
		efree(haltname);
		return (ret == SUCCESS);
	} else {
		return 0;
	}
}
/* }}} */

/* {{{ zend_quick_get_constant */
zend_constant *zend_quick_get_constant(const zend_literal *key, ulong flags TSRMLS_DC)
{
	zend_constant *c;

	if (zend_hash_quick_find(EG(zend_constants), Z_STRVAL(key->constant), Z_STRLEN(key->constant) + 1, key->hash_value, (void **) &c) == FAILURE) {
		key++;
		if (zend_hash_quick_find(EG(zend_constants), Z_STRVAL(key->constant), Z_STRLEN(key->constant) + 1, key->hash_value, (void **) &c) == FAILURE ||
		    (c->flags & CONST_CS) != 0) {
			if ((flags & (IS_CONSTANT_IN_NAMESPACE|IS_CONSTANT_UNQUALIFIED)) == (IS_CONSTANT_IN_NAMESPACE|IS_CONSTANT_UNQUALIFIED)) {
				key++;
				if (zend_hash_quick_find(EG(zend_constants), Z_STRVAL(key->constant), Z_STRLEN(key->constant) + 1, key->hash_value, (void **) &c) == FAILURE) {
				    key++;
					if (zend_hash_quick_find(EG(zend_constants), Z_STRVAL(key->constant), Z_STRLEN(key->constant) + 1, key->hash_value, (void **) &c) == FAILURE ||
					    (c->flags & CONST_CS) != 0) {

						key--;
						if (!zend_get_special_constant(Z_STRVAL(key->constant), Z_STRLEN(key->constant), &c TSRMLS_CC)) {
							return NULL;
						}
					}
				}
			} else {
				key--;
				if (!zend_get_special_constant(Z_STRVAL(key->constant), Z_STRLEN(key->constant), &c TSRMLS_CC)) {
					return NULL;
				}
			}
		}
	}
	return c;
}
/* }}} */

/* {{{ DeepTrace_constant_handler */
int DeepTrace_constant_handler(ZEND_OPCODE_HANDLER_ARGS)
{
	DEEPTRACE_DECL_STRING_PARAM(className);
	DEEPTRACE_DECL_STRING_PARAM(constName);
	DEEPTRACE_DECL_STRING_PARAM(combinedName);
	DEEPTRACE_DECL_STRING_PARAM(cacheName);
	void *cachePtr;
	zend_constant *constant;
	ulong hash;

	/* Get name and length of constant */
	constName = Z_STRVAL(EX(opline)->op2.literal->constant);
	constName_len = Z_STRLEN(EX(opline)->op2.literal->constant);

	/* Create new cache if necessary */
	if(!DEEPTRACE_G(constantCache)) {
		ALLOC_HASHTABLE(DEEPTRACE_G(constantCache));
		zend_hash_init(DEEPTRACE_G(constantCache), 4, NULL, NULL, 0);
	}

	/* Check if it is an class constant or not */
	if(EX(opline)->op1_type == IS_UNUSED) {
		hash = zend_inline_hash_func(constName, constName_len + 1);

		/* Check if constant was already cached */
		if(zend_hash_quick_find(DEEPTRACE_G(constantCache), constName, constName_len + 1,
				hash, (void **) &cachePtr) == FAILURE) {
			constant = zend_quick_get_constant(EX(opline)->op2.literal + 1, EX(opline)->extended_value TSRMLS_CC);
			cachePtr = (void *) constant;

			/* Add constant to cache */
#ifdef DEEPTRACE_DEBUG_CONSTANT_CACHE
			zend_printf("[DT Constant Cache] Add> %s @ %d\n", constName, cachePtr);
#endif
			zend_hash_quick_add(DEEPTRACE_G(constantCache), constName, constName_len + 1, hash,
					(void **) &cachePtr, sizeof(void *), NULL);
		} else {
			/* Get pointer from cache */
			cachePtr = *((void **) cachePtr);
#ifdef DEEPTRACE_DEBUG_CONSTANT_CACHE
			zend_printf("[DT Constant Cache] Get> %s @ %d\n", constName, cachePtr);
#endif
		}
	} else {
		/* Ignore polymorphic constants */
		if(EX(opline)->op1_type != IS_CONST) {
			EG(active_op_array)->run_time_cache[EX(opline)->op2.literal->cache_slot] = 0;
			return ZEND_USER_OPCODE_DISPATCH;
		}

		/* Build constant name and get hash */
		className = Z_STRVAL_P(EX(opline)->op1.zv);
		className_len = Z_STRLEN_P(EX(opline)->op1.zv);
		combinedName = emalloc(className_len + constName_len + 2);
		combinedName_len = className_len + constName_len + 1;
		sprintf(combinedName, "%s%c%s", className, '\0', constName);

		/* Build cache name and get hash */
		char *tmp = zend_str_tolower_dup(className, className_len);
		cacheName = emalloc(className_len + constName_len + 3);
		cacheName_len = className_len + constName_len + 2;
		sprintf(cacheName, "%s::%s", tmp, constName);
		hash = zend_inline_hash_func(cacheName, cacheName_len + 1);
		efree(tmp);

		/* Check if constant is already cached */
		if(zend_hash_quick_find(DEEPTRACE_G(constantCache), cacheName, cacheName_len + 1,
				hash, (void **) &cachePtr) == FAILURE) {
			zend_class_entry *ce;
			zval **value;

			/* Get class pointer */
			if(CACHED_PTR(EX(opline)->op2.literal->cache_slot)) {
				value = CACHED_PTR(EX(opline)->op2.literal->cache_slot);
				ZVAL_COPY_VALUE(&EX_T(EX(opline)->result.var).tmp_var, *value);
				zval_copy_ctor(&EX_T(EX(opline)->result.var).tmp_var);

				efree(combinedName);
				return ZEND_USER_OPCODE_DISPATCH;
			} else if (CACHED_PTR(EX(opline)->op1.literal->cache_slot)) {
				ce = CACHED_PTR(EX(opline)->op1.literal->cache_slot);
			} else {
				ce = zend_fetch_class_by_name(className, className_len,
						EX(opline)->op1.literal + 1, EX(opline)->extended_value TSRMLS_CC);
				if (UNEXPECTED(ce == NULL)) {
					efree(combinedName);
					return ZEND_USER_OPCODE_DISPATCH;
				}
				CACHE_PTR(EX(opline)->op1.literal->cache_slot, ce);
			}

			/* Get constant pointer */
			if(zend_hash_quick_find(&ce->constants_table, constName, constName_len + 1,
					Z_HASH_P(EX(opline)->op2.zv), (void **) &value) == FAILURE) {
				EG(active_op_array)->run_time_cache[EX(opline)->op2.literal->cache_slot] = 0;
				return ZEND_USER_OPCODE_DISPATCH;
			}
			cachePtr = (void *) value;

			/* Add constant to cache */
#ifdef DEEPTRACE_DEBUG_CONSTANT_CACHE
			zend_printf("[DT Constant Cache] Add> %s @ %d\n", cacheName, cachePtr);
#endif
			zend_hash_quick_add(DEEPTRACE_G(constantCache), cacheName, cacheName_len + 1,
					hash, (void**) &cachePtr, sizeof(void*), NULL);
		} else {
			/* Get pointer from cache */
			cachePtr = *((void **) cachePtr);
#ifdef DEEPTRACE_DEBUG_CONSTANT_CACHE
			zend_printf("[DT Constant Cache] Get> %s @ %d\n", cacheName, cachePtr);
#endif
		}

		efree(combinedName);
		efree(cacheName);
	}

	EG(active_op_array)->run_time_cache[EX(opline)->op2.literal->cache_slot] = cachePtr;
	return ZEND_USER_OPCODE_DISPATCH;
}
/* }}} */

/* {{{ DeepTrace_constants_cleanup */
void DeepTrace_constants_cleanup()
{
	if(DEEPTRACE_G(constantCache)) {
		zend_hash_destroy(DEEPTRACE_G(constantCache));
		FREE_HASHTABLE(DEEPTRACE_G(constantCache));
		DEEPTRACE_G(constantCache) = NULL;
	}
}
/* }}} */
