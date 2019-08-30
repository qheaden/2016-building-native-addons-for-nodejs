#include <node.h>

namespace node_cpp_tutorial {
  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::String;
  using v8::Value;
  using v8::Exception;
  using v8::Number;
  using v8::Context;

  void AddTwoNumbers(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (args.Length() < 2) {
      isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "You must pass two numbers.")));
      return;
    }

    if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "You must pass in only number values.")));
      return;
    }

    double returnValue = args[0]->ToNumber(context).ToLocalChecked()->Value() + args[1]->ToNumber(context).ToLocalChecked()->Value();

    args.GetReturnValue().Set(Number::New(isolate, returnValue));
  }

  void AddMultipleNumbers(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (args.Length() < 2) {
      isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "You must pass at least two numbers.")));
      return;
    }

    double returnValue = 0;

    for(int i = 0; i < args.Length(); i++) {
      if (!args[i]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "You must pass in only number values.")));
        return;
      }

      returnValue += args[i]->ToNumber(context).ToLocalChecked()->Value();
    }

    args.GetReturnValue().Set(Number::New(isolate, returnValue));
  }

  void Initialize(Local<Object> exports) {
    NODE_SET_METHOD(exports, "addTwoNumbers", AddTwoNumbers);
    NODE_SET_METHOD(exports, "addMultipleNumbers", AddMultipleNumbers);
  }

  NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)
}
