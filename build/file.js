{
print("1");
}
{
}
var a = 1;
if(a == 1){
	print("go in to");
}else{
	print(error);
}
if( a == 1)
	print("tw");
function hello(){
	return "hello world";
}
print(hello());
for(var i = 0 ; i < 3; ++i){
	if(i == 0)
		continue;
	print("i");
	if(i == 2)
		break;

}
try{
	throw "TEST";
}catch(e){
	print(e);

}
var a = {a:1,b:3};

for(var name in a){
	print(name);
}


this.a = "Global a";
print(this.a);
var array = ["a","b","c","d"];
print("In array");
for(var i = 0 ; i < array.length; ++i){
	print(array[i]);
}
var o = function(a,b,c){
	this.a = a;
	this.b = b;
	this.c = c;
};
o.prototype = {S:"S"};
var o1 = new o("1","2",{});
for(var name in o1){

	print(o1[name]);
	
}
print(Object.prototype.toString.apply(array));