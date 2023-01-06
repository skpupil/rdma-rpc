#include "route.h"
class server {
  route router_;
public:
  template <typename Function>
  void register_handle(string name, Function f) {
    router_.register_func_outter(name ,f);
  }
  void read_body(string name) {
  	router_._route(name);
  }
};

int adder(int a,int b) {
	return a + b;
}
int main() {
	server ser;
	ser.register_handle("addder", adder);
	ser.read_body("addder");
	return 0;

}
