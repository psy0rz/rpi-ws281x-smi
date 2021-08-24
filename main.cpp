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

    color_t color;

    color.component.r=(uint32_t )info[2].As<Napi::Number>();
    color.component.g=(uint32_t )info[3].As<Napi::Number>();
    color.component.b=(uint32_t )info[4].As<Napi::Number>();
    color.component.a=(uint32_t )(((float )(info[5].As<Napi::Number>()))*255); //every extra parameter takes about 10fps in our 16*256 led test.

    leds_set_pixel(channel, pixel, color);
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
