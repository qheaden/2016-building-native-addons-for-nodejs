#include <node.h>

namespace node_cpp_tutorial {
  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::String;
  using v8::Value;

  void GetHello(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "Hello World!"));
  }

  void Initialize(Local<Object> exports) {
    NODE_SET_METHOD(exports, "getHello", GetHello);
  }

  NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)
}
