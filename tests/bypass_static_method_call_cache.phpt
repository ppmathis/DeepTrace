--TEST--
Bypassing static method call cache
--SKIPIF--
<?php
if(!extension_loaded("DeepTrace")) print "skip";
if(!function_exists("dt_destroy_class_data")) print "skip";
if(!function_exists("dt_remove_class")) print "skip";
?>
--FILE--
<?php
	class A {
	        public function doIt() {
	                echo B::getData();
	        }
	}
	 
	class B {
	        private static $x = 7;
	        public static function getData() {
	                return self::$x;
	        }
	}
	 
	$x = new A();
	$x->doIt();
	
	dt_destroy_class_data('B');
	dt_remove_class('B');
	class B {
	        private static $x = 7;
	 
	        public static function getData() {
	                return self::$x;
	        }
	}
	
	$x->doIt();
?>
--EXPECT--
77