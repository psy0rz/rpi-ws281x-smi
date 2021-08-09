#include <napi.h>
#include <cstdio>
#include "smileds.h"

void ledsInit(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    int ledCount = info[0].As<Napi::Number>();
    if (!leds_init(ledCount))
    {
        Napi::TypeError::New(env, "Failed").ThrowAsJavaScriptException();
    }
}

/**
 * Usage: ledSetPixel(channel, ledNr, rgbColor)
 * @param info
 */
void ledsSetPixel(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    int channel = info[0].As<Napi::Number>();
    int pixel = info[1].As<Napi::Number>();
    uint32_t rgb = info[2].As<Napi::Number>();

    leds_set_pixel(channel, pixel, rgb);
}

void ledsSend(const Napi::CallbackInfo& info) {
    leds_send();
}

void ledsClear(const Napi::CallbackInfo& info) {
    leds_clear();
}




static Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports["init"] = Napi::Function::New(env, ledsInit);
    exports["send"] = Napi::Function::New(env, ledsSend);
    exports["clear"] = Napi::Function::New(env, ledsClear);
    exports["setPixel"] = Napi::Function::New(env, ledsSetPixel);

  return exports;

}


NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
