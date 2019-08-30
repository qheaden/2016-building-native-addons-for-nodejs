

#include <node.h>
#include <uv.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace node_cpp_tutorial {
  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Context;
  using v8::Local;
  using v8::Persistent;
  using v8::Object;
  using v8::String;
  using v8::Value;
  using v8::Exception;
  using v8::Number;
  using v8::Null;
  using v8::Function;
  using v8::HandleScope;
  using v8::NewStringType;

  struct WorkInfo {
      uv_work_t request;
      uint32_t sleepTime;
      Persistent<Function> jsCallback;
  };

  void SleepCallback(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (args.Length() < 2) {
        isolate->ThrowException(
          Exception::Error(
            String::NewFromUtf8(isolate, "You must pass in the sleep time and a callback function.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    if (!args[0]->IsUint32()) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Your first argument must be an integer.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    if (!args[1]->IsFunction()) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Your second argument must be a callback function.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    uint32_t sleepTime = args[0]->ToUint32(context).ToLocalChecked()->Value();

#ifdef _WIN32
    Sleep(sleepTime);
#else
    usleep(sleepTime * 1000);
#endif

    Local<Function> callback = Local<Function>::Cast(args[1]);
    callback->Call(context, Null(isolate), 0, nullptr);
  }

  void SleepAsync(uv_work_t* request) {
    WorkInfo* info = static_cast<WorkInfo*>(request->data);

#ifdef _WIN32
    Sleep(info->sleepTime);
#else
    usleep(info->sleepTime * 1000);
#endif
  }

  void SleepAsyncComplete(uv_work_t* request, int status) {
      Isolate* isolate = Isolate::GetCurrent();

      HandleScope handleScope(isolate);
      Local<Context> context = isolate->GetCurrentContext();

      WorkInfo* info = static_cast<WorkInfo*>(request->data);
      info->jsCallback.Get(isolate)->Call(context, Null(isolate), 0, nullptr);
  }

  void SleepCallbackAsync(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (args.Length() < 2) {
        isolate->ThrowException(
          Exception::Error(
            String::NewFromUtf8(isolate, "You must pass in the sleep time and a callback function.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    if (!args[0]->IsUint32()) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Your first argument must be an integer.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    if (!args[1]->IsFunction()) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Your second argument must be a callback function.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    WorkInfo* info = new WorkInfo();
    info->sleepTime = args[0]->ToUint32(context).ToLocalChecked()->Value();
    info->request.data = info;

    Local<Function> callback = Local<Function>::Cast(args[1]);
    info->jsCallback.Reset(isolate, callback);

    uv_queue_work(uv_default_loop(), &info->request, SleepAsync, SleepAsyncComplete);
  }

  void Initialize(Local<Object> exports) {
    NODE_SET_METHOD(exports, "sleepCallback", SleepCallback);
    NODE_SET_METHOD(exports, "sleepCallbackAsync", SleepCallbackAsync);
  }

  NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)
}
