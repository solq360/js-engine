(function(){
	var a =1 ,b =2 ,c =3;
	print("============test var==========");
	print(a);
	print(b);
	print(c);
	print("===============================");
	print("============test ;==========");
	;;;;;
	print("=============================");
	print("============test if==========");
	if(  a == 1)
		print("a = 1");
	else
		print(" a != 1");
	if( a != 1)
		print(" a != 1")
	else
		print(" a == 1");
	print("============test if==========");
	print("============test for var==========");
	for(var i = 0 ; i < arguments.length; ++i)
		print(arguments[i]);
	print("==================================");
	print("============test do while==========");
	var i = 0;
	do{
		i++;
		print(i);
		//break;
	}while(i <= 3);
	print("=======================================");
	print("===============test while =============");
	
	var i = 0;
	while(i <= 3){
		print(i);
		i++;
	}
	print("=======================================");
	print("===============test in =============");
	var obj = {a:1,b:'hello'};
	for(var name in obj){
		print("obj "+name+": "+obj[name]);
	}
	print("===============================");
	print("============== test continue break============");
	for(var i = 0 ; i < 100;++i){
		if(i < 10)
			continue;
		print(i);
		break;
	}
	print("===============================");
	print("============test return=========");
	var c  = function(){
		return "succ";
	};
	print(c());
	print("===============================");
	print("====== test try catch throw ===");
	try{
		print("into try");
		throw "error";
	}catch(e){
		print("into catch");
		print(e);
	}finally{
		print("into finally");
	}
	print("===============================");
	print("====== test function  ===");
	function hello(a,b,c){
		print(a+b+c);
	}
	hello("hello",2,"function");
	var func = function(a,b,c){
		for(var i = 0; i < arguments.length ; ++i){
			print(arguments[i]);
		}
		print(c);
	}
	func(1,2);
	print("===============================");
	print("============test prototype===========");
	var Class = function(name){
		this["name"] = name;
	};
	Class.prototype = {soup : 21};
	var C1 = new Class("C1");
	var C2 = new Class("C2");
	print("C1-name: "+C1.name);
	print("C2-name: "+C2.name);
	for(var name in  Class.prototype)
		print(name);
	print("C1-soup: " +C1.soup);
	print("C2-soup: "+  C2.soup);
	Class.prototype.soup = 12;
	//print("C1-soup: " +C1.soup);
	print("C2-soup: "+  C2.soup);
	print("change C1 soup");
	C1.soup = "c1";
	print("C1-soup: "+C1.soup);
	print("C2-soup: "+C2.soup);
	print("===============================");
	print("===========test clouse=========");
	var test1 = {
		hello:'hello',
		good:'good'
	};
	var test2 = {};
	for(var name in test1){
		test2[name] = (function(name){
			return function(){
				print(name);
			}
		})(name);
	}
	test2['hello']();
	test2['good']();
	
	print("===============================");
	print("===========test exception ,======");
	var a = 2;
	print(a + 1),print(a+2);
	print("===============================");
	print("===========test assignment =============");
	var c = 10;
	print("c = 10 : "+ c);
	print("c *=2 : "+ (c*=2));
	print("c /= 2: "+ (c/=2) );
	print("c %= 3: " + (c%=3) );
	print("c += 9: " + (c +=9));
	print("c -= 10:" + (c -=10));
	print("===============================");
	print("=========test exp?exp:exp============");
	var c =1;
	c ? print("c = succ") : print("error");
	print("===============================");
	print("=============test bin logical==========");
	if( false && true){
		print("error");
	}else{
		print("succ");
		
	}
	if( true && true){
		print("succ");
	}else{
		print("error");
		
	}
	if(false || false){
		print("error");
	}else{
		print("succ");
	}
	print("===============================");
	print("=======test equality Operators=============");
	var c = {};
	var d = c;
	if( c == d)
		print("object ref succ");
	var c = "abc";
	var d = "abc";
	if( c==d){
		print("string eq succ");
	}
	d = "abcd";
	if( c!= d){
		print("string != succ");
	}
	print("===============================");
	print("=======test relational Operators=============");
	print("string <" + ("abc" < "abcd"));
	print("string >" + ("abcd" > "abc"));
	print("string <=" + ("abc" <= "abc"));
	print("string >=" + ("abc" >= "abc"));
	print("number <" + (1 < 2));
	print("number >" + (2 > 1));
	print("number <=" + (1 <= 1));
	print("number >=" + (1 >= 1));
	var a = {a:1};
	print("in: " + ('a' in a));
	print("instanceof: " + (C1 instanceof Class));
	print("===============================");
	print("=======test Additive Operators =============");
	print("1+3:"+ (1 + 3));
	print("1-3:"+ (1 - 3));
	print("===============================");
	print("=======test Multiplicative Operators =============");
	print("12*3:"+ (12 * 3));
	print("12%3:"+ (12 % 3));
	print("12/3:"+ (12 / 3));
	print("===============================");
	print("=========Unary===========");
	var a = 1;
	print("delete don't delete: " + (delete a) == false);
	abcd =1;
	print("delete :" + (delete abcd));
	print("void: "+ ((void a)==undefined));
	print("typeof");
	var a = [null,undefined,1,'string',{},true,false];
	for(var i = 0 ; i <  a.length ; ++ i){
		print(typeof a[i]);
	}
	i = 1;
	print("++i, i =1 :" + (++i));
	print("--i, i =2 :" + (--i));
	print("!i, i =1 : "+ !i);
	i = NaN;
	print("!i, i =NaN : "+ !i);
	i = 0;
	print("!i, i =0 : "+ !i);
	i = undefined;
	print("!i, i = undefined : "+ !i);
	print((NaN-1) == NaN);
	print("===============================");
	print("============test postfix================");
	var i= 0;
	print("i++,i=0"+ ( i++));
	print("i--,i=1"+(i--));
	
})(0,1,2,3,"string",true,false,{a:1},function(){});

(function(){
	//test Base API
	print("=====================test Base API===========");
	print(NaN);
	print(undefined);
	eval("print(this)");
	print(isNaN(1));
	print(isNaN(NaN));
	
	
	var c = new Object(1);
	for(var name in c ){
		print(name);
	}
	print(c);
	c.a = 1;
	print(c.hasOwnProperty('a'));
	print(c.hasOwnProperty('c'));
	print(c.hasOwnProperty('hasOwnProperty'));
	var  c = function(){};
	var d = new c();
	print(c.prototype.isPrototypeOf(d));
	var array = new Array(10,1,21,2);
	for(var i = 0;i<array.length;++i)
		print(array[i]);
	array.push(1);
	array.pop();
	print(array);
	
	
	var string = new String("12");
	print(string);
	string = new String(32);
	print(string);
	string   = "abcd";
	print(string.length);
	for(var i = 0; i < string.length;++i){
		print(string.charAt(i))
	}
	var b = new Boolean(true);
	print(b);
	
	var a = new Number("1233");
	print(a);
})();
setTimeout(function(){
	print("hello setTimeout");
	throw "finish";
},5);

synchronized function funa(id){
	for(var i = 0 ; i < 100;++i)
		print("thread ["+id+"]: "+i);
	throw "finish"
}
var i = 0;
var lock = {};
var fCreateNewThread = function(id,fun){
	thread(function(){
		synchronized(lock){
			for(var i = 0 ; i < 10;++i)
				print("thread ["+id+"]: "+i);
			throw "finish"
		}
		funa(id);
	});
};
for(var i = 0 ; i < 5 ;++i)
	fCreateNewThread(i);
	