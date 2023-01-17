#include "server.hh"
#include "hello.pb.h"

auto main([[gnu::unused]] int argc, char *argv[]) -> int {
  rdma::Server s(argv[1], argv[2]);

  s.registerHandler(0, [](rdma::RPCHandle &handle) -> void {
    echo::Hello request;
    handle.getRequest(request);
    printf("receive request: \"%d %d %d\"\n", request.addr(),request.arg1(),request.arg2());
    echo::Hello response;
    auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
    //response.set_greeting("hi from server thread " + std::to_string(thread_id));
    response.set_addr(10098);
    printf("set response: \"%d\"\n", response.addr());
    handle.setResponse(response);
  });

  return s.run();
}
