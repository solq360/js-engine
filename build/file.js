(function(){
	for(var i = 0 ; i < arguments.length; ++i)
		print(arguments[i]);
	delete arguments;
	 a = 1;
	delete a;
	print(typeof a);

})(0,1,2,3,"string",true,false,{a:1},function(){});
eval("print('hello world')");
