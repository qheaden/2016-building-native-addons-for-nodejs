// node.h contains all of the function definitions needed to interface with
// Node and the V8 runtime as well as registration functions.
#include <node.h>
#include <uv.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace node_cpp_tutorial {
  // Typing v8 in front of everything takes up a lot of extra space. We
  // want to bring these V8 objects directly into our namespace.
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

  // This structure is used to keep important data available to the code running on different
  // threads during asyncronous sleep.
  struct WorkInfo {
      uv_work_t request; // This is a handle reference to the libuv work request that was created.
      uint32_t sleepTime; // This is a copy of the sleep time parameter that the user will pass into the function.
      Persistent<Function> jsCallback; // This is a reference to the JS callback function that will fire after sleep.
  };

  void SleepCallback(const FunctionCallbackInfo<Value>& args) {
    // The Isolate pointer representing the V8 JS engine instance.
    Isolate* isolate = args.GetIsolate();

    // We need to get a reference to the execution context which manages our JS objects,
    // functions, etc. Instead of getting a direct pointer to the context, we are provided
    // a Local<> smart pointer. Objects wrapped in a Local<> smart pointer are memory-manged
    // by the V8 runtime. It is best practice to always handle V8 objects with smart pointer
    // wrapper classes.
    Local<Context> context = isolate->GetCurrentContext();

    // If we have fewer than two arguments passed to the JS function, throw an exception.
    if (args.Length() < 2) {
        isolate->ThrowException(
          Exception::Error(
            String::NewFromUtf8(isolate, "You must pass in the sleep time and a callback function.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    // If the first argument passed in JS is not an unsigned integer, throw an exception.
    if (!args[0]->IsUint32()) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Your first argument must be an integer.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    // If the second argument passed to the JS function is not another JS function, throw an exception.
    if (!args[1]->IsFunction()) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Your second argument must be a callback function.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    // We need to read in the sleep time given to the JS function, and convert it to an unsigned integer C++ type.
    uint32_t sleepTime = args[0]->ToUint32(context).ToLocalChecked()->Value();

    // We call the sleep function to pause the process for the specified amount of time. The ifdef block
    // is there since the sleep functions on Windows and Linux are different.
#ifdef _WIN32
    Sleep(sleepTime);
#else
    usleep(sleepTime * 1000);
#endif

    // We get a reference to the callback function that was provided from JS as the second parameter to
    // the function call. We then call it using the v8::Function::Call() method. The first parameter to
    // this method call is the V8 execution context. The second parameter passes in an object that holds
    // the return value from JS. Since this JS callback has no return value, we pass a V8 Null here. The
    // third parameter is the number of arguments we are sending to the JS callback, which is zero in this
    // case. The fourth parameter is a pointer to a list of v8::Value objects as parameters, Since we are
    // sending no params, we pass a null pointer here.
    Local<Function> callback = Local<Function>::Cast(args[1]);
    callback->Call(context, Null(isolate), 0, nullptr);
  }

  void SleepAsync(uv_work_t* request) {
    // We retrieve the WorkInfo object we told libuv to pass to this function once it
    // was called. Remember that this structure has, among other things, our custom
    // sleep time value.
    WorkInfo* info = static_cast<WorkInfo*>(request->data);

    // We call the sleep function to pause the process for the specified amount of time. The ifdef block
    // is there since the sleep functions on Windows and Linux are different.
#ifdef _WIN32
    Sleep(info->sleepTime);
#else
    usleep(info->sleepTime * 1000);
#endif
  }

  void SleepAsyncComplete(uv_work_t* request, int status) {
      Isolate* isolate = Isolate::GetCurrent();

      // Since we are in a different execution thread, we need to create a new HandleScope and
      // associate it with this Isolate execution environment so that smart pointers to the V8
      // resources can be properly managed.
      HandleScope handleScope(isolate);
      Local<Context> context = isolate->GetCurrentContext();

      // We retrieve the WorkInfo object we told libuv to pass to this function once it
      // was called. Remember that this structure has, among other things, a reference
      // to our callback function in the V8 engine.
      WorkInfo* info = static_cast<WorkInfo*>(request->data);

    // We get a reference to the callback function that was provided from JS as the second parameter to
    // the function call. We then call it using the v8::Function::Call() method. The first parameter to
    // this method call is the V8 execution context. The second parameter passes in an object that holds
    // the return value from JS. Since this JS callback has no return value, we pass a V8 Null here. The
    // third parameter is the number of arguments we are sending to the JS callback, which is zero in this
    // case. The fourth parameter is a pointer to a list of v8::Value objects as parameters, Since we are
    // sending no params, we pass a null pointer here.
      info->jsCallback.Get(isolate)->Call(context, Null(isolate), 0, nullptr);
  }

  void SleepCallbackAsync(const FunctionCallbackInfo<Value>& args) {
    // The Isolate pointer representing the V8 JS engine instance.
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    // If we have fewer than two arguments passed to the JS function, throw an exception.
    if (args.Length() < 2) {
        isolate->ThrowException(
          Exception::Error(
            String::NewFromUtf8(isolate, "You must pass in the sleep time and a callback function.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    // If the first argument passed in JS is not an unsigned integer, throw an exception.
    if (!args[0]->IsUint32()) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Your first argument must be an integer.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    // If the second argument passed to the JS function is not another JS function, throw an exception.
    if (!args[1]->IsFunction()) {
        isolate->ThrowException(
          Exception::TypeError(
            String::NewFromUtf8(isolate, "Your second argument must be a callback function.", NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    // We create a new WorkInfo structure. This struct will hold a list of values we want
    // to pass to our asyncronous function that will be called by libuv.
    WorkInfo* info = new WorkInfo();
    info->sleepTime = args[0]->ToUint32(context).ToLocalChecked()->Value();
    info->request.data = info;

    // We need to ensure the JS callback object is properly tracked so we can actually call it
    // once libuv finishes the work in another thread.
    Local<Function> callback = Local<Function>::Cast(args[1]);
    info->jsCallback.Reset(isolate, callback);

    // Now that our data is set up, we tell libuv we want to kick off the C++ asychronous sleep
    // function we defined above without blocking the main thread. Once the work in that function
    // is complete, we also tell libuv to call SleepAsyncComplete.
    uv_queue_work(uv_default_loop(), &info->request, SleepAsync, SleepAsyncComplete);
  }

  void Initialize(Local<Object> exports) {
    // We register the C++ functions as methods to this addon. The second argument to
    // NODE_SET_METHOD is a string that defines what the method name will be in Node.js,
    // and the third argument is the name of the C++ function that will be called. It
    // is not required that they be the same, but it is good practice.
    NODE_SET_METHOD(exports, "sleepCallback", SleepCallback);
    NODE_SET_METHOD(exports, "sleepCallbackAsync", SleepCallbackAsync);
  }

  // This is a C++ macro provided by node.h that registers the addon with
  // the V8 runtime, making it avaialble to Node.js when require'd in the code.
  NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)
}
