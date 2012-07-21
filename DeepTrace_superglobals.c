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

#ifdef DEEPTRACE_CUSTOM_SUPERGLOBALS

/* {{{ dt_get_superglobal
	Get a array of all existent superglobals */
PHP_FUNCTION(dt_get_superglobals) {
	HashPosition pos;
	char *sg;
	int sg_len, type;
	long idx;

	array_init(return_value);
	for(zend_hash_internal_pointer_reset_ex(CG(auto_globals), &pos);
		(type = zend_hash_get_current_key_ex(CG(auto_globals), &sg, &sg_len, &idx, 0, &pos)) != HASH_KEY_NON_EXISTANT;
		zend_hash_move_forward_ex(CG(auto_globals), &pos)) {
		if (type == HASH_KEY_IS_STRING) {
			add_next_index_stringl(return_value, sg, sg_len - 1, 1);
		}
	}
}
/* }}} */

/* {{{ dt_register_superglobal
	Register a new superglobal */
int dt_register_superglobal(char* variableName, int len) {
	ulong h;

	/* Get hash */
	h = zend_inline_hash_func(variableName, len + 1);

	/* Check if already registered */
	if(zend_hash_quick_exists(CG(auto_globals), variableName, len + 1, h)) {
		return 0;
	}

	/* Register auto global */
	zend_auto_global auto_global;

	auto_global.name = (char*) zend_new_interned_string((char*) variableName, len + 1, 0 TSRMLS_CC);
	auto_global.name_len = len;
	auto_global.auto_global_callback = NULL;
#	if DT_PHP_VERSION == 54
		auto_global.jit = 1;
#	endif
	zend_hash_quick_add(CG(auto_globals), variableName, len + 1, h, &auto_global, sizeof(zend_auto_global), NULL);
	zend_rebuild_symbol_table();

	return 1;
}
/* }}} */

#endif
