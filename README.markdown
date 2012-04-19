# DeepTrace v1.3
### Copyright (c) 2012 P. Mathis (pmathis@snapserv.net)
* * *
#### License informations
This code is licensed via a Creative Commons Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/

Means:

* You may alter the code, but have to give the changes back
* You may not use this work for commercial purposes
* You must attribute the work in the manner specified by the author or licensor

If you like to use this code commercially, please contact me via pmathis@snapserv.net. Thank you!

#### What is DeepTrace?
DeepTrace is a small and neat extension for different low level things in PHP. You can catch exit()/die(), remove and rename functions, remove and redefine constants and much more.

#### How could I install it?
Just do a few easy steps. First, download the complete project. After that, run these commands:

*phpize*

*./configure*

*sudo make*

*sudo make install*

Now you only need to add the following entry into your php.ini:

*zend_extension=DeepTrace.so*

Got some problem? Ask google for it and if you still can not find any solution, just contact me. 

**Important:**
DeepTrace is a zend extension, so do not use *extension=DeepTrace.so*!

#### What could I do with it?
You could do different things with it. Here are the most of the functions in a short and easy list:

* Overwrite internal or userdefined functions
* Remove internal or userdefined functions
* Redefine and remove internal or userdefined constants (It is even possible to modify constants like PHP_SAPI, PHP_VERSION and so on!)
* Handle exit() / die() calls without exiting
* Include scripts with exit() / die() calls. If there is one, the include gets aborted but the main script continues running.
* Remove classes (experimental)
* Set process title (for threaded applications useful)
* Remove includes from internal hash table (require_once is then possible multiple times)

#### Documentation?!? I can not find anyone! HELP!
No problem. Take a look into the "DeepTrace.php" files in the examples directory. It shows all the functions of the DeepTrace extension and how you could use them.

#### I want to include that library in my project
That is not a problem for me, but there are a few restrictions:

* If you change some code, you MUST give the chances back to me
* I would be glad if you inform me
* You must not remove any copyrights or license informations

If you want to include the library in a compiled form, ask me via pmathis@snapserv.net - I am not a bad person, I just wanna know who uses my extension :)

#### There is something missing!
No problem, just say it and if the idea is good I will implement it. Reason why you should use this? It does not eat much ressources and you will only need one extension.

#### Real examples
[Pancake - Small and fast PHP powered webserver](https://github.com/pp3345/Pancake "Made by pp3345")

You did something great with DeepTrace? Tell it to me and maybe it will land here :-)
