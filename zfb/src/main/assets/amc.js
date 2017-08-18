/*
 * Alipay.com Inc.
 * Copyright (c) 2004-2015 All Rights Reserved.
 *
 * common.js
 *
 */

// 名字空间 alipay mobile cashier
var amc = {};

amc.rpcData = flybird.rpcData;
amc.platform = document.platform || 'chrome';
amc.isIOS = (amc.platform === 'iOS');
amc.isAndroid = (amc.platform === 'android');
amc.isFlybird = (amc.isIOS || amc.isAndroid);

amc.isPlus = false;
if(amc.isIOS){
    amc.isPlus = (document.getCustomAttr('isIphonePlus') === 'true');
    amc.osVersion = document.getCustomAttr('osVersion');
}

if(amc.isFlybird) {
    amc.sdkVersion = document.getCustomAttr('sdkVersion') || '0.0.0';
}

amc.path =  amc.isAndroid ? 'com.alipay.android.app/' : 'AlipaySDK.bundle/';

// display='flex'使隐藏元素可见
amc.VISIBLE = 'flex';
// 公共方法名字空间 function
amc.fn = {};
amc.fn.getById = function(id) {
    return document.getElementById(id);
};

/**
 * 版本比较
 * 备注:合法的版本号只包含点号和数字，每个点号间用数字分隔(10..0属不合法);
 *     如果两个版本号长度不同(1.0与1.0.9),短的补零进行比较(1.0.0与1.0.9)
 * v1(string) 待比较版本
 * v2(string) 待比较版本
 * return: 1)NaN: v1,v2任意一个不合法 2)1: v1 > v2 3) 0: v1 = v2 4)-1: v1 < v2
 */
amc.fn.versionCompare = function(v1, v2) {
    v1 = amc.fn.isString(v1) ? v1 : '';
    v2 = amc.fn.isString(v2) ? v2 : '';
    
    
    v1parts = v1.split('.'),
    v2parts = v2.split('.');
    
    function isValidPart(x) {
        return (/^\d+$/).test(x);
    }
    
    if (!v1parts.every(isValidPart) || !v2parts.every(isValidPart)) {
        return NaN;
    }
    
    while (v1parts.length < v2parts.length) v1parts.push('0');
    while (v2parts.length < v1parts.length) v2parts.push('0');
    
    
    v1parts = v1parts.map(Number);
    v2parts = v2parts.map(Number);
    
    for (var i = 0; i < v1parts.length; ++i) {
        
        if (v1parts[i] == v2parts[i]) {
            continue;
        } else if (v1parts[i] > v2parts[i]) {
            return 1;
        } else {
            return -1;
        }
    }
    
    return 0;
};

amc.fn.sdkGreaterThan = function(v){
    return (amc.fn.versionCompare(amc.sdkVersion, v) === 1);
};

/**
 * SDK 10.7.0开始支持
 */
amc.fn.sdkGreaterThanOrEqual = function(v){
    return (amc.fn.versionCompare(amc.sdkVersion, v) === 1 || amc.fn.versionCompare(amc.sdkVersion, v) === 0);
};
/**
 * 埋点方法
 * biz(string) 埋点类型：1. 'promotion' 广告类 2.
 * info(object) 埋点参数
 */
amc.fn.feedback = function(biz, info) {
    if(!info) {
        return;
    }
    
    var paramObj = {};
    var feedbackType = biz || 'promotion';
    paramObj[feedbackType] = info;
    
    var actionName = "loc:feedback('" + JSON.stringify(paramObj) + "')";
    var obj = {'action':{'name':actionName}};
    document.submit(obj);
};

/**
 * 支持版本: 钱包9.9.6, sdk 10.6.6
 * 记录action日志
 * 对应日志字段: initial,[actionType],[action],41,529,1038,277,155,QUICKPAY@open-pwd-check-flex,1,-,169,2,13:08:40:787),
 */
amc.fn.logAction = function(action, actionType) {
    if (!action) return;

    var paramObj = {};
    paramObj['name'] = action;
    paramObj['type'] = actionType||'tpl-click';
    
    var actionName = "loc:continue;loc:log('" + JSON.stringify({'action': paramObj}) + "')";
    var obj = {'action':{'name':actionName}};
    document.submit(obj);
}

/**
 * 支持版本: 钱包9.9.6, sdk 10.6.6
 * 记录错误日志
 * 日志字段: ([code],[desc])
 */
amc.fn.logError = function(code, desc) {
    if (!code || !desc) return;

    var paramObj = {};
    paramObj['code'] = code;
    paramObj['desc'] = desc;
    
    var actionName = "loc:continue;loc:log('" + JSON.stringify({'error': paramObj}) + "')";
    var obj = {'action':{'name':actionName}};
    document.submit(obj);
}

/**
 * 支持版本: 钱包10.0.0, sdk 10.7.0才提供该方法
 * 获取当前模板名称，如: cashier-activity-flex
 * flybird.local作为Native传递本地信息的对象
 */
amc.fn.getTplId = function() {
    if(amc.fn.sdkGreaterThan('10.6.9') && flybird && flybird.local) {
        return flybird.local.tplId || '';
    } else {
        return '';
    }
};

/**
 * 打印日志:记录次数, 支持版本: 钱包9.9.6, sdk 10.6.6
 * 备注: biz/op/desc均按照自己分析日志的需求制定，日志分析联系:@子学
 * 日志字段: (@@count@@[biz],[op],[desc],-),
 */
amc.fn.logCount = function(biz, op, desc) {
    if (!biz || !op) return;

    var paramObj = {};
    paramObj['biz'] = biz;
    paramObj['op'] = op;
    paramObj['desc'] = desc;
    
    var actionName = "loc:continue;loc:log('" + JSON.stringify({'count': paramObj}) + "')";
    var obj = {'action':{'name':actionName}};
    document.submit(obj);
}

/**
 * 支持版本: 钱包9.9.6, sdk 10.6.6
 * 上报页面init函数执行完毕
 * 日志字段: (@@count@@tpl,init,[tplId],-),
 */
amc.fn.logPageInit = function(success) {
    amc.fn.logCount('tpl', !!success ? 'init-success' : 'init-fail', amc.fn.getTplId());
}

/**
 * 支持版本: 钱包9.9.6, sdk 10.6.6
 * 设置返回结果。
 * 备注: result['doNotExit'] 表示 不退出收银台
 */
amc.fn.setResult = function(result) {
    if(!result) {
        return;
    }

    document.submit({'action':{'name': 'loc:returnData', 'params': result}});
}

/*
 * 支持版本: 钱包9.9.6, sdk 10.6.6
 * 提交页面结果
 */
amc.fn.submitData = function(resultPageData) {
    if(!resultPageData) {
        return;
    }
    
    var result = {resultPageData: resultPageData, doNotExit: true};
    amc.fn.setResult(result);
}

/*
 * 创建节点
 * tagName(string): 要创建的节点的类型名称，如'div'
 * className(string): CSS类名
 * parent(tag): 父节点
 */
amc.fn.create = function(tagName, className, parent){
    var tag = document.createElement(tagName);
    if(!tag){
        return;
    }
    
    if(className) {
        tag.className = className;
    }
    
    if(parent) {
        parent.appendChild(tag);
    }
    
    return tag;
};

/*
 * 创建节点
 * data(obj): data['pageloading'] === '1'或'0',分别表示显示indicator和隐藏indicator
 * btnTitleId(string): 需要被隐藏或显示的按钮文案的id
 * indicatorId(string): 需要被显示或隐藏的indicator图片的id
 */
amc.fn.shouldShowBtnIndicator = function(data, btnTitleId, indicatorId) {
    if(!data) {
        return false;
    }
    
    if(data['pageloading'] === '1'){
        amc.fn.hide(btnTitleId);
        amc.fn.show(indicatorId);
        return true;
    } else if(data['pageloading'] === '0') {
        amc.fn.show(btnTitleId);
        amc.fn.hide(indicatorId);
    }
    
    return false;
};

// 公共图片资源名字空间 resource
amc.res = {};
//全屏返回按钮
amc.res.navBack = amc.path + 'alipay_msp_back';
amc.res.navClose = amc.path + 'alipay_msp_back_close';
amc.res.navMore = amc.path + 'alipay_msp_mini_three_point';
//半屏返回按钮
amc.res.arrowLeft = amc.path + 'alipay_msp_arrow_left';
amc.res.close = amc.path + 'alipay_msp_close';
amc.res.arrowRight = amc.path + 'alipay_msp_arrow_right';
// 菊花
amc.res.loading = amc.path + 'alipay_msp_loading.gif';
amc.res.success = amc.path + 'alipay_msp_success.gif';

amc.res.help = amc.path + 'alipay_msp_help';
//小菊花
amc.res.indicator = 'indicatior';

amc.specs = {};
amc.specs.navHeight =  amc.isAndroid ? 48 : 64; // iPhone 状态栏高度均为 64 point
amc.specs.marqueeHeight = 36;
//安卓中body包含导航栏、iOS中body不包含导航栏
amc.specs.bodyHeight = window.innerHeight - amc.specs.navHeight;
amc.specs.iBodyMinH = amc.isPlus ? 518 : 400;
amc.specs.iBodyHeight = (0.66 * window.innerHeight > amc.specs.iBodyMinH) ? Math.round(0.66 * window.innerHeight) : amc.specs.iBodyMinH;
amc.specs.iNavHeight = amc.isPlus ? 60 : 46; //45.5的高度+1px的横线

amc.fn.exit = function() {
    document.submit({'action':{'name':'loc:exit'}});
};
amc.fn.back = function () {
    document.submit({'action':{'name':'loc:back'}});
};

/*
 * 在外部(浏览器或淘宝等)打开链接(在支付过程中，如果在支付过程中跳转会影响支付成功率)
 * url(string): 要跳转的地址
 * willReturn(string):
 * '0': 退出收银台，不给业务方结果
 * '1': 退出收银台，给业务方结果
 * '2': 不退出收银台，不给业务方结果
 * '3': 退出收银台，且可恢复
 */
amc.fn.openurl = function(url, returnType) {
    if(!url){
        return;
    }

    var returnTypes = ['0','1','2','3'];
    if(!(returnType in returnTypes)) {
        returnType = '0';
    }
    
    document.submit({'action':{'name':"loc:openurl('"+ url +"','" + returnType + "')"}});
};

/*
 * 在内置浏览器打开链接(如:打开协议内容)
 * url(string): 要打开的链接地址
 */
amc.fn.openweb = function(url) {
    if(!url){
        return;
    }
    document.submit({'action':{'name':"loc:openweb('"+url+"')"}});
};

amc.fn.getMarquee = function(txt) {
    var marquee = create('marquee');
    marquee.className = 'amc-marquee';
    marquee.innerText = txt || '';
    return marquee;
};
//### 播放gif图片
amc.fn.playGif = function(gifNode,gifImg){
    if (gifImg){
        gifNode.src = gifImg;
        amc.fn.show(gifNode);
    } else {
        amc.fn.hide(gifNode);
    }
};

amc.fn.hideKeyboard = function() {
    //### 收起全部键盘
    var inputArray = document.querySelectorAll('input');
    for (var i = 0; i < inputArray.length; i++) {
        inputArray[i].blur();
    }
};
//### 退出确认
amc.fn.exitConfirm = function(msg){
    msg = msg || '{{confirm_exit}}';
    var confirmObj = {
        'title': '',
        'message': msg,
        'okButton': '{{confirm_btn}}',
        'cancelButton': '{{cancel}}'
    };
    
    document.confirm(confirmObj, function (result) {
        if (result.ok) {
            document.submit({"action":{"name":"loc:exit"}});

        }
    });
};

//### 协议个数不定时打开协议
amc.fn.showProtocolList = function(pList) {
    if(!pList || pList.length <= 0){
        return;
    }
    // 一个协议
    if (pList.length == 1){
        document.submit({"action":{"name":"loc:openweb('"+pList[0]["url"]+"','"+pList[0]["text"]+"')"}});
        return;
    }
    // 多个协议
    var protocolArray = new Array();
    for (var i = 0; i < pList.length; i++) {
        protocolArray.push(pList[i]["text"]);
    }
    document.actionSheet({
            'text' : '{{protocol}}',
            'btns': protocolArray,
            'cancelBtn': '{{cancel}}'
            }, function(data) {
                if(data.index < pList.length) {
                    document.submit({"action":{"name":"loc:openweb('"+pList[data.index]["url"]+"','"+pList[data.index]["text"]+"')"}});
                }
        }
    );
};

amc.fn.isString = function(str) {
    return ((str instanceof String) || typeof str === 'string');
};

amc.fn.isArray = function(o) {
    return Object.prototype.toString.call(o) === '[object Array]';
};

amc.fn.isObject = function(o) {
    return Object.prototype.toString.call(o) === '[object Object]';
};

amc.fn.show = function(tag)
{
    if(tag){
        tag = amc.fn.isString(tag) ? document.getElementById(tag) : tag;
        
        if(tag){
            tag.style.display = amc.VISIBLE;
        }
    }
};

amc.fn.hide = function(tag)
{
    if(tag){
        tag = amc.fn.isString(tag) ? document.getElementById(tag) : tag;
        
        if(tag){
            tag.style.display = 'none';
        }
    }
};

/*
 * 视图范例: < 返回     支付宝      完成/···  (/表示互斥)
 * lImg(string): 左侧图片(通常作为返回按钮), 默认id=“navImgL”;false则不创建
 * lTxt(string): 最左侧的文字(“返回”):如果为undefined/""则不创建;默认id="navTxtL";
 * mTxt(string): 中间文案(作为Title):如果为undefined/""则不创建;
 * rTxt(string): 最左侧的文字(“完成”/"取消"等):默认id="navTxtR"; 如果为undefined/""则不创建
 * rImg(string): 最右侧的图片(例如:作为"more"按钮),默认id="navImgR"; false则不创建
 * onLeft(function): 左侧box按下的回调
 * onRight(function): 右侧box按下的回调
 * option(object): 手势返回参数 钱包10.0.0才支持
 */
amc.fn.iOSNav = function(lImg, lTxt, mTxt, rTxt,  rImg, onLeft, onRight, option)
{
    var create = amc.fn.create;
    
    // nav
    var _nav = create('nav');
    _nav.className = 'amc-nav-box';
    
    // left box
    var _lBox = create('div');
    _lBox.className = 'amc-nav-l-box';
    _lBox.id = 'navBoxL';

    // 左滑返回手势(仅适用于iOS,返回按钮)，默认状态是禁用
    var eb = create('embed', 'amc-hidden', _lBox);
    eb.type = 'MQPGestureBack';
    
    // left img
    if(lImg){
        var _lImg = create('img');
        _lImg.className = 'amc-nav-l-img';
        _lImg.id = 'navImgL';
        _lImg.src = lImg;
        _lBox.appendChild(_lImg);
        
        //voice over
        if(lImg === amc.res.navBack){
           _lBox.alt = '{{return}}';
 
            eb.onfocus = onLeft;
            eb.src = 'gesture';
            if (option && option.backMode) {
                eb.src = option.backMode;
            }
        }
        _lBox.onclick = onLeft;
    }
    
    // left label
    if(lTxt){
        var _lTxt = create('label');
        _lTxt.className = 'amc-nav-l-text';
        _lBox.alt = lTxt;
        
        //如果左侧没有图片，则不需要间距
        if(!lImg) {
            _lTxt.style.marginLeft = 0;
        }
        _lTxt.id = 'navTxtL';
        _lTxt.innerText = lTxt;
        _lBox.appendChild(_lTxt);
        
        _lBox.onclick = onLeft;
    }
    
    // middle box
    var _mBox = create('div');
    _mBox.className = 'amc-nav-m-box amc-v-box';
    // 6p: 两边padding 40, l-box/r-box 各84, 总和208;
    // 5s: padding 30, l-box/r-box 各70 总和 170
    // 算上间距，稍微多减一点
    _mBox.style.maxWidth = window.innerWidth - (amc.isPlus ? (210) : (180));
    if(mTxt){
        var _mTxt = create('label');
        _mTxt.id = 'navTxtM';
        _mTxt.innerText = mTxt;
        _mTxt.className = 'amc-nav-m-text';
        _mBox.appendChild(_mTxt);
    }
    
    var _rBox = create('div');
    _rBox.className = 'amc-nav-r-box';
    _rBox.id = 'navBoxR';
    
    
    if(rTxt){
        var _rTxt = create('label');
        _rTxt.className = 'amc-nav-r-text';
        _rTxt.innerText = rTxt;
        _rTxt.id = 'navTxtR';
        _rBox.appendChild(_rTxt);
        _rBox.onclick = onRight;
    } else if(rImg){
        var _rImg = create('img');
        _rImg.className = 'amc-nav-r-img'
        _rImg.id = 'navImgR';
        _rImg.src = rImg;
        _rBox.appendChild(_rImg);
        _rBox.onclick = onRight;
    }
    
    _nav.appendChild(_lBox);
    _nav.appendChild(_mBox);
    _nav.appendChild(_rBox);

    return _nav;
};


/*
 * 视图范例: < | 返回       ···
 * 视图范例: 支付宝         |完成
 * lImg(string): 左侧图片(通常作为返回按钮), 默认id=“navImgL”;
 * lTxt(string): 安卓没有左侧文字,该变量作为占位符; 该参数为Null
 * mTxt(string): 中间文案(作为Title):
 * rTxt(string): 最右侧的文字(“完成”/"取消"等):默认id="navTxtR";
 * rImg(string): 最右侧的图片(例如:作为"more"按钮),默认id="navImgR"; 
 * onLeft(function): 左侧box按下的回调
 * onRight(function): 右侧box按下的回调
 * 注意: 以上参数若为''/undefined/false/null则不创建
 */
amc.fn.androidNav = function(lImg, lTxt, mTxt, rTxt, rImg, onLeft, onRight)
{
    var create = amc.fn.create;
    
    // 导航栏外容器(为了在导航栏下方放置横线)
    var _navContainer = create('div', 'amc-nav-container-android');
    
    // 导航栏
    var _nav = create('div', 'amc-nav-box-android', _navContainer);
    // 导航栏下方横线
    var line = create('div', 'amc-nav-horiz-line-android', _navContainer);
    
    
    var _lBox = create('div', 'amc-nav-l-box-android', _nav);
    _lBox.id = 'navBoxL';
    
    // <
    if(lImg){
        var _lImgBox = create('div', 'amc-nav-l-img-box-android', _lBox);
        _lImgBox.onclick = onLeft;
        _lImgBox.id = 'navImgBoxL';
        
        var _lImg = create('img', 'amc-nav-l-img-android', _lImgBox);
        _lImg.id = 'navImgL';
        _lImg.src = lImg;
        
        // |
        var _lBar = create('div', 'amc-nav-line-android', _lBox);
        
        //voice over
        if(lImg === amc.res.navBack){
            _lImg.alt = '{{return}}';
        } else if(lImg === amc.res.navClose){
            _lImg.alt = '{{exit}}';
        }
    }
    
    // 返回 or 支付宝
    var _mTxt = create('label', 'amc-nav-m-text-android', _lBox);
    _mTxt.innerText = mTxt || '';
    _mTxt.id = 'navTxtM';
    
    
    var _rBox = create('div');
    _rBox.className = 'amc-nav-r-box-android';
    _rBox.id = 'navBoxR';
    
    // ···
    if(rImg){
        var _rImg = create('img', 'amc-nav-r-img-android', _rBox);
        _rImg.id = 'navImgR';
        _rImg.src = rImg;
        
        _rBox.onclick = onRight;
    } else if(rTxt) {
        // | 设置
        var _rBar = create('div', 'amc-nav-line-android', _nav);
        
        var _rTxt = create('label', 'amc-nav-r-text-android', _rBox);
        _rTxt.id = 'navTxtR';
        _rTxt.innerText = rTxt;
        _rBox.onclick = onRight;
    }
    _nav.appendChild(_rBox);
    
    //安卓平台需要有nav标签才能能判断是全屏
    var _isFullScreen = create('nav', 'amc-hidden', _navContainer);
    
    return _navContainer;
};

amc.fn.getNav = amc.isAndroid ? amc.fn.androidNav : amc.fn.iOSNav;
amc.fn.pressableElement = function(elDiv,el) {
    if(elDiv && el){
        elDiv.onmousedown = function(){
            el.style.opacity = '0.5';
        };
        elDiv.onmouseup = function() {
            el.style.opacity = '1';
        };
    }
};

/*
 * isBack 左上角为返回按钮、否则为取消(安卓为关闭)
 * title(string): 中间文案(作为Title),id: iNavTxtM
 * rTxt(string): 最左侧的文字(“完成”/"取消"等):
 * rImg(string): 最右侧的图片(例如:作为"more"按钮):默认id="inavImgR";
 * onLeft(function): 左侧box按下的回调
 * onRight(function): 右侧box按下的回调
 * 注意: 以上参数若为''/undefined/false/null则不创建
 */
amc.fn.navBack = function(isBack, title, rTxt, rImg, onLeft, onRight, option){
    if(isBack){
        return amc.fn.getNav(amc.res.navBack, '{{return}}', title, rTxt, rImg, onLeft, onRight, option);
    } else {
        return amc.fn.getNav(amc.isAndroid ? amc.res.navClose : null, '{{cancel}}', title, rTxt, rImg, onLeft, onRight, option);
    }
};


/*
 * 视图范例: x    付款详情   ?/设置
 * lImg(string): 左侧按钮图片
 * mTxt(string): 中间文案(作为Title),id: iNavTxtM
 * rTxt(string): 最左侧的文字(“完成”/"取消"等):
 * rImg(string): 最右侧的图片(例如:作为"more"按钮):默认id="inavImgR";
 * onLeft(function): 左侧box按下的回调
 * onRight(function): 右侧box按下的回调
 * 注意: 以上参数若为''/undefined/false/null则不创建
  * mImg pre-confirm页特殊需求，需要添加一个img
 */
amc.fn.iNav = function(lImg, mTxt, rTxt, rImg, onLeft, onRight, mImg) {
    var create = amc.fn.create;
    var _iNav = create('div');
    _iNav.className = 'amc-i-nav-box';
    
    var _lBox = create('div');
    _lBox.className = 'amc-i-nav-l-box';
    _lBox.id = 'iNavBoxL';
    
    if(lImg) {
        var _lImg = create('img');
        _lImg.className = 'amc-i-nav-l-img';
        _lImg.src = lImg;
        _lImg.id = 'iNavImgL';
        _lBox.appendChild(_lImg);
        amc.fn.pressableElement(_lBox,_lImg);
        
        if(lImg === amc.res.arrowLeft){
            _lBox.alt = '{{return}}';
        }else if(lImg === amc.res.close){
            _lBox.alt = '{{exit}}';
        }
        _lBox.onclick = onLeft;
    }
    
    var _mBox = create('div');
    _mBox.className = 'amc-i-nav-m-box';
    if(mImg){
        var _mImg = create('img');
        _mImg.id = 'iNavImgM';
        _mImg.src = mImg;
        _mImg.className = 'amc-i-nav-m-img';
        _mBox.appendChild(_mImg);
    }
    
    if(mTxt){
        var _mTxt = create('label');
        // 防止icon被文案挤到最左边
        if(mImg) {
            _mTxt.className = 'amc-i-nav-m-text amc-ellipsis';
            _mTxt.style.maxWidth = window.innerWidth / 2 - 50;
        } else {
            _mTxt.className = 'amc-i-nav-m-text amc-ellipsis amc-flex-1';
        }
        
        _mTxt.innerText = mTxt;
        _mTxt.id = 'iNavTxtM';
        _mBox.appendChild(_mTxt);
    }
    var _rBox = create('div');
    _rBox.className = 'amc-i-nav-r-box';
    _rBox.id = 'iNavBoxR';
    
    if(rImg) {
        var _rImg = create('img');
        _rImg.className = 'amc-i-nav-r-img';
        _rImg.id = 'iNavImgR';
        _rImg.src = rImg;
        _rBox.appendChild(_rImg);
        amc.fn.pressableElement(_rBox,_rImg);
        _rBox.onclick = onRight;
    } else if(rTxt) {
        var _rTxt = create('label');
        _rTxt.className = 'amc-i-nav-r-text amc-flex-1 amc-ellipsis';
        _rTxt.id = 'iNavTxtR';
        _rTxt.innerText = rTxt;
        _rBox.appendChild(_rTxt);
        amc.fn.pressableElement(_rBox,_rTxt);
        _rBox.onclick = onRight;
    }
    
    _iNav.appendChild(_lBox);
    _iNav.appendChild(_mBox);
    _iNav.appendChild(_rBox);
    
    return _iNav;
};



/**
 * 根据key值获取国际化文案对应的value值
 * @param key(string). 国际化语言的key值,形如:{{key}}/key
 * @return (string) 返回国际化语言对应的value
 */
amc.fn.i18nValueForKey = function(key) {
    if(!key) {
        return key;
    }

    var tmpKey = key;
    if(key.indexOf('{{') < 0) {
        tmpKey = '{{' + key + '}}';
    }
    
    var global = document;
    global.i18nInvisibleTag = global.i18nInvisibleTag || amc.fn.create('label');
    
    if(!global.i18nInvisibleTag)
    {
        return key;
    }
    
    // 将key值赋值给UI控件时进行国际化语言替换
    document.i18nInvisibleTag.innerText = tmpKey;
    
    return global.i18nInvisibleTag.innerText;
};

/*
 * 进行占位符替换，将字符串中的#key1#替换成对应的value1.
 * @param str(string) 被替换的字符串
 * @param keyValue(object/string) 键值对/单值:
 * 1. 例: function('总数为#count#，总金额为#amount#', {count: 20, amount: '100.0'});
 * 2. 如果只有一个替换值的形式可以简写成: function('总数为#count#', 20);
 */
amc.fn.keyValueReplace = function(str, keyValue){

    if(!str || !keyValue) {
        return str;
    }
    
    // 如果只有一个替换值时，keyValue可以简写成该值，此时被替换字符为#val#
    if(!amc.fn.isObject(keyValue)) {
        keyValue = {'val': keyValue}
    }
    
    for(var key in keyValue) {
        str = str.replace(new RegExp('#' + key + '#', "g"), keyValue[key]);
    }
    
    return str;
};

/*
 * 进行占位符替换
 * @param key(string) 国际化语言的key值
 */
amc.fn.i18nPlaceholderReplace = function(key, keyValue) {
    return amc.fn.keyValueReplace(amc.fn.i18nValueForKey(key), keyValue)
};

/**
 * 从中间将文字截断(从钱包10.0.0， SDK 10.6.9开始支持)
 * str(string) 需要截断的字符串
 * headNum(number) 头部保留字数
 * tailNum(number) 末尾保留字数
 */
amc.fn.stringTruncate = function(str, headNum, tailNum) {
    if(!str || (!headNum && !tailNum)) {
        return str;
    }
    
    if(isNaN(headNum) || headNum < 0) {
        headNum = 0;
    }
    
    if(isNaN(tailNum) || tailNum < 0) {
        tailNum = 0;
    }
    
    if(headNum + tailNum >= str.length) {
        return str;
    }
    
    var head = str.substr(0, headNum);
    str = str.substr(headNum, str.length - headNum);
    
    var tail = '';
    tail = str.substr(str.length - tailNum);
    
    return head + '...' + tail;
};

/**
 * 渲染本地模板
 * @param  {string} tplId   模板ID, 如:QUICKPAY@open-pwd-check-flex
 * @param  {object} tplMeta 模板信息，由服务端下发
 * @param  {object} data    模板数据, 在目标模板内通过rpc关键词访问
 */
amc.fn.renderLocalTemplate = function(tplId, tplMeta, data) {
    if (!tplId) {
        return;
    }

    var obj = {};
    obj['action'] = { 'name': 'loc:bnvb' };
    obj['param'] = {};

    obj['param']['tplid'] = tplId;
    obj['param']['tpl'] = tplMeta;
    obj['param']['data'] = data;

    document.submit(obj);
};

/**
 * 弹出Native 弹窗
 *
 * @param  {Function} callback 回调函数，回调入参为点击index, 从0开始, 与传入btns数组序号一致
 * @param  {Array} btns 按钮文案数组
 * @param  {string} message 提示文案
 * 备注:  不支持<font color=''></font>标签 投放方需要注意
 */
amc.fn.nativeAlert = function(btns, callback, message) {
    if (!btns || !btns.length || !(typeof callback === 'function')) {
        return;
    }

    // Native弹窗配置信息
    var dialogConfig = {
        // title: title,   // 只有iOS支持
        message: message || ''
    };

    
    
    // 3个按钮时候，iOS显示顺序依次为: 按钮2、按钮3、按钮1(粗体)
    // 在这里进行一次转换
    
    var needResort = amc.isIOS && btns == 3;
    if(needResort) {
        var tmp = btns[2];
        btns[2] = btns[1];
        btns[1] = btns[0];
        btns[0] = tmp;
    }
    
    // 一个按钮
    if (btns.length >= 1) {
        dialogConfig['cancelButton'] = btns[0] || '{{cancel}}';
    }

    // 两个按钮
    if (btns.length >= 2) {
        dialogConfig['okButton'] = btns[1] || '{{confirm_btn}}';
    }

    // 三个按钮
    if (btns.length >= 3) {
        dialogConfig['otherButton'] = btns[2] || '其他';
    }

    var obj = { 'action': { 'name': "loc:alert('" + JSON.stringify(dialogConfig) + "')" } };

    document.asyncSubmit(obj, function(result) {
        if (callback && result && !isNaN(result.index)) {                         
            var index = result.index;
            if (needResort) {
                if (index == 2) {
                    index = 1;
                } else if (index == 1) {
                    index = 0;
                } else {
                    index = 2;
                }
            }
                         
            callback(index);

            // 安卓中可能会重复多次回调callback, 为避免发生重复操作, 调用一次之后清除回调
            callback = null;
        }
    });
};

/*
 * 用于鸟巢页面感知页面的生命周期，页面中可覆盖此方法
 * 备注: 每次从被遮挡到显示都会回调
 * 仅适用于iOS，从SDK 10.7.3，钱包10.0.8开始支持
 */
document.viewDidAppear = function(){};

/*
 * 覆盖鸟巢提供的document.confirm方法
 * 参数参考: http://birdnest.alipay.net/books/js/#confirm.html
 * 本方法在钱包9.6.9之后才能支持
 */
;(function(){
     document.confirm = function (btn, callback) {
         if(!btn || !callback
            || !(typeof btn === 'object')
            || !(typeof callback === 'function')) {
             return;
         }
 
         var obj = {'action' : {'name': "loc:alert('" + JSON.stringify(btn)  + "')"}};
         
         document.asyncSubmit(obj, function(result) {
              callback(result);
        });
     }
})();
