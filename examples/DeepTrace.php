<?php
	/*
		+-----------------------------------------------------------------------+
		| DeepTrace v1.2 ( Homepage: https://www.snapserv.net/ )			 	|
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

	/*
		+-----------------------------------------------------------------------+
		| Test: dt_rename_function												|
		+-----------------------------------------------------------------------+
		| Parameters: string oldFunctionName, newFunctionName 					|
		| Return value: bool success											|
		+-----------------------------------------------------------------------+
		| Explanation: After this test, the internal function phpinfo 			|
		| calls your declared replacement function. If you need 				|
		| to call the original function at anytime, just call					|
		| internal_phpinfo()													|
		+-----------------------------------------------------------------------+
	*/
	function my_phpinfo() {
		echo "phpinfo was disabled for security reasons (btw: this message is user-generated!)\n";
	}

	dt_rename_function("phpinfo", "internal_phpinfo");	// Rename phpinfo() function to internal_phpinfo()
	dt_rename_function("my_phpinfo", "phpinfo");		// Rename my_phpinfo() to phpinfo()
	phpinfo();											// Call phpinfo() (leads to my_phpinfo)

	/*
		+-----------------------------------------------------------------------+
		| Test: dt_remove_function												|
		+-----------------------------------------------------------------------+
		| Parameters: string functionName 										|
		| Return value: bool success											|
		+-----------------------------------------------------------------------+
		| Explanation: It is simple. This function removes any other function -	|
		| it does not matter if the function is internal or userdefined. But 	|
		| you should be careful with internal functions. You should always give |
		| them some replacement.												|
		+-----------------------------------------------------------------------+
	*/
	dt_remove_function("header");						// Remove the internal function header()

	if(function_exists("header")) {
		echo "Function header() exists\n";				// If you see this, something did not work.
	} else {
		echo "Function header() doesn't exist\n";		// If you see this, everything is working!
	}

	/*
		+-----------------------------------------------------------------------+
		| Test: dt_remove_constant												|
		+-----------------------------------------------------------------------+
		| Parameters: string constantName 										|
		| Return value: bool success											|
		+-----------------------------------------------------------------------+
		| Explanation: It is simple. This function removes any user-defined		|
		| constant. It does not work for internal constants, because it is 		|
		| dangerous and sometimes even impossible to do so.						|
		+-----------------------------------------------------------------------+
	*/
	define("WRONG_CONST", "I was wrong.");

	dt_remove_constant("WRONG_CONST");					// Remove the user defined constant
	if(defined("WRONG_CONST")) {
		echo "Constant WRONG_CONST is defined.\n";		// If you see this, something did not work.
	} else {
		echo "Constant WRONG_CONST is not defined.\n";	// If you see this, everything is working!
	}

	/*
		+-----------------------------------------------------------------------+
		| Test: dt_set_proctitle												|
		+-----------------------------------------------------------------------+
		| Parameters: string procTitle 											|
		| Return value: bool success											|
		+-----------------------------------------------------------------------+
		| Explanation: This function sets the process title. This is useful 	|
		| for multithreaded PHP applications, because then you can give all 	|
		| your forked threads some pretty names. Try it out!					|
		+-----------------------------------------------------------------------+
	*/
	dt_set_proctitle("DeepTrace powered!");				// This sets the process title to DeepTrace, you could
														// see that with ps -aux. But because this application
														// ends instantly,you should add some while(1) {} to
														// really see this function working

	/*
		+-----------------------------------------------------------------------+
		| Test: dt_set_exit_handler												|
		+-----------------------------------------------------------------------+
		| Parameters: string functionName										|
		| Return value: bool success											|
		+-----------------------------------------------------------------------+
		| Explanation: Everybody knows that there are 2 php commands: die and 	|
		| exit. They are both the same and they kill the script instantly. If 	|
		| you are creating some sort of sandbox for a script, that function 	|
		| could be really useful. It will redirect every upcoming exit/die 		|
		| call to your function. But that isn't all. What happens after calling |
		| your function? You can choose:										|
		|																		|
		| If you return true, the original exit handler would get called and 	|
		|		the script exits.												|
		| If you return false, the script will just continue running without	|
		| 		any problems. 													|
		|																		|
		| This function is only useful if you know what you wanna do with it - 	|
		| otherwise it can be pretty painful, it is not a magic bug fixer for 	|
		| every script on the world.											|
		|																		|
		| IMPORTANT: Your function must accept an optional parameter (string).	|
		| The default value should be set to NULL. DeepTrace passes the exit 	|
		| message to this parameter.											|
		+-----------------------------------------------------------------------+
	*/
	function my_exit_handler($reason = NULL) {
		echo "You called exit()/die() with the following reason: \n";
		echo $reason . "\n";

		return false;									// The script should continue running
	}

	dt_set_exit_handler("my_exit_handler");				// Handle exit() & die() with your own function
	exit("Normally this would murder your script.");	// Call exit() - but the script does not stop, thanks to DeepTrace
	die("That would be deadly too.");					// Call die() - same as above ;-)

	/*
		+-----------------------------------------------------------------------+
		| Test: dt_throw_exit_exception											|
		+-----------------------------------------------------------------------+
		| Parameters: bool enableThrowing										|
		| Return value: bool success											|
		+-----------------------------------------------------------------------+
		| Explanation: This function only works, when you have set some custom	|
		| error handler. It changes the workflow of the custom exit handler:	|
		|																		|
		| Exit handler returns true -> original handler, script is dead 		|
		| Exit handler returns false -> 										|
		|		enableThrowing == true -> Exit handler throws some exception 	|
		|			Exception message is always DeepTraceExitException			|
		|		enableThrowing == false -> Script continues                     |
		|																		|
		| This makes it possible to include some file in a sandboxed way.		|
		| If there is any exit() in the included file, the include will be 		|
		| aborted but the main script still continues running. That could be 	|
		| pretty neat for things like PHP Webservers. ( Sounds incredible, but 	|
		| there are really some projects like this. Take a look at Pancake for 	|
		| example: https://github.com/pp3345/Pancake )							|
		+-----------------------------------------------------------------------+
	*/
	dt_throw_exit_exception(true);

	try {
		// Include here any files with exit()
		// Run anything with exit() inside
		// The possibilities are endless

		// That exit command will throw an exception with the message "DeepTraceExitException"
		exit("That command could be in a include file - it would still work.");		
	} catch (Exception $e) {
		echo "try/catch block aborted. Exception: " . $e->getMessage() .  "\n";
	}

	/*
		+-----------------------------------------------------------------------+
		| Test: dt_remove_include												|
		+-----------------------------------------------------------------------+
		| Parameters: string includeName										|
		| Return value: bool success											|
		+-----------------------------------------------------------------------+
		| Explanation: Removes an include from the internal hash map. Also 		|
		| only useful if you know what you do with it. With the help of this 	|
		| magical function you could include a file multiple times - with the   |
		| functions include_once & require_once. Also pretty neat if you wanna	|
		| do some sandboxed environment. (Like said above, Pancake is a good 	|
		| example for that - And no, I will not get any money for saying things |
		| like this! :P
		+-----------------------------------------------------------------------+
	*/

	require_once("include.php");						// require_once() with include.php
	dt_remove_include("include.php");					// Delete include.php from hash map
	require_once("include.php");						// require_once() with include.php (and it works again!)

	/*
		+-----------------------------------------------------------------------+
		| Test: dt_remove_class													|
		+-----------------------------------------------------------------------+
		| Parameters: string className											|
		| Return value: bool success											|
		+-----------------------------------------------------------------------+
		| Explanation: Removes an entire class with all its constant, variables |
		| and methods.															|
		|																		|
		| IMPORTANT: Highly experimental. Could cause memory leaks and bad 		|
		| things like this. Only for use in development areas.					|
		+-----------------------------------------------------------------------+
	*/
	class DeepTraceRocks {
		function test() {
			echo "hi";
		}
	}

	dt_remove_class("DeepTraceRocks");
	if(class_exists("DeepTraceRocks")) {
		echo "Class DeepTraceRocks exists.\n";
	} else {
		echo "Class DeepTraceRocks does not exist.\n";
	}
?>
