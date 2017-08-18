/*
 * Alipay.com Inc.
 * Copyright (c) 2006-2020 All Rights Reserved.
 *
 * mic_common.js
 *
 */

// 名字空间 MobileIC
var mic = {};

mic.fn = {};

mic.fn.onBackWithResponse = function onBackWithResponse(data) {
    print("[VILog]onBackWithResponse !!!");
    obj = {};
    obj['eventName'] = 'vi_quit_module_with_response';
    obj['params'] = data;
    document.submit(obj);
};

mic.fn.onBack = function onBack() {
    print("[VILog]onback !!!");
    obj = {};
    obj['eventName'] = 'vi_quit_module';
    document.submit(obj);
};

mic.fn.changeModule = function changeModule() {
    print("[VILog]changeModule");
    obj = {};
    obj['eventName'] = 'vi_change_module';
    document.submit(obj);   
};
