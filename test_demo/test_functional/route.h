#include "iostream"
#include "string"
#include "map"
#include "functional"
using namespace std;

struct dummy {
	static int add(int a,int b, int c) {
		return a + b;
	}

};


template <typename Function>
struct my_struct {

	static int add(int a,int b, int c) {
		return a + b;
	}
	//template <typename Function>
};


class route {
	map<string, std::function<void(char*, size_t, string& ) > > func_map;
	int class_add(int a,int b) {
		return a + b;
	}
	//}
public:
	void _route(string name) {
		char* ch = "d";size_t zt = 12;string str = "add";
		auto itt = func_map.find(name);
		itt->second(ch,zt,str);
		//func_map[name](ch,zt,str);
	}
	template <typename Function>
	void register_func_outter(const string& name ,Function f){
		register_func(name, f);
	}
	//template <typename Function, typename Slef>
	//void register_func(const string& name ,const Function& f, Slef *slef ) {
	//  func_map[name] = {std::bind(&f,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)};
	//}
	
//template <typename Function>
template <typename Function>
struct invoker {
	static void applyy(const Function& f,char* a ,size_t b, string& c) {
		//auto ret = (*f)(1,2);
		printf("this is called applyy\n");

	}
};
template <typename Function>
void register_func(const string& name ,Function f) {
	func_map[name] = {std::bind( &invoker<Function>::template applyy, f, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)};
	auto funcc_apply = std::bind(&invoker<Function>::template applyy,f,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	char* ch = "dd";
	size_t zt = 1;
	string ss = "d";
	funcc_apply(ch,zt,ss);
	
	//auto funcc = std::bind(&f,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//int ret = funcc(1,2,3);
	//printf("%d", ret);
}
};
/*
class server {
  route router_;
  template <typename Function>
  void register_handle(string name, Function f) {
    router_.register_func(name ,f);
  }
};
*/
/*
int main() {
	printf("start register");
	dummy du;
	//invoker in;
	//auto funcc_apply = std::bind(&applyy,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//invoker<Function>::register_func("add",dummy::add);
	printf("end register");
	return 0;
}
*/
