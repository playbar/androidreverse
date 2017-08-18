
var userAgent = navigator.userAgent.toLowerCase();
var isFlybird = userAgent.indexOf("flybird") >= 0; //### 是否为飞鸟引擎
var isAndroid = userAgent.indexOf("android") >= 0; //### 是否为android平台
var isIOS = (isFlybird && (!isAndroid));

//### 资源路径
var path = isAndroid ? "com.alipay.android.app/" : "AlipaySDK.bundle/";


//### 服务器传来的参数大都赋值给新变量:
//### 1.压缩空间:变量名可以在压缩过程中被替换成较短的字符
//### 2.避免在后文中出现级联的"."操作,导致产生空引用访问错误
var rpcData = flybird.rpcData;

//### 缩减代码量
function getById(id) {
    return document.getElementById(id);
}

function create(tagName){
    return document.createElement(tagName);
}

//### 退出确认
function exitConfirm(msg){
    msg = msg || '确定要退出吗?';
    var confirmObj = {
        title: '',
        message: msg,
        okButton: '确定',
        cancelButton: '取消'
    };
    ant.confirm(confirmObj, function (result) {
        if (result.ok) {
            ant.submit({"action":{"name":"loc:exit"}});
        }
    });
}

//### 按钮点击效果
function pressEffect(item){
    var color = item.style.backgroundColor;
    item.onmousedown = function () {
        item.style.transition = 0;
        item.style.backgroundColor = "#e5e5e5";
    }
    item.onmouseup = function() {
        item.style.backgroundColor = color;
        item.style.transition = 0.475;
    }
}

//### 协议个数不定时打开协议
function showProtocolList(pList) {
    //### 一个协议
    if (pList.length == 1){
        ant.submit({"action":{"name":"loc:openweb('"+pList[0]["url"]+"','"+pList[0]["text"]+"')"}});
        return;
    }
    //### 多个协议
    var protocolArray = new Array();
    for (var i = 0; i < pList.length; i++) {
        protocolArray.push(pList[i]["text"]);
    }
    ant.actionSheet({
            'btns': protocolArray,
            'cancelBtn': '取消'
        }, function(data) {
            ant.submit({"action":{"name":"loc:openweb('"+pList[data.index]["url"]+"','"+pList[data.index]["text"]+"')"}});
        }
    );
}