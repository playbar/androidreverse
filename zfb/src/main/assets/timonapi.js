function createActionBean() {
    return {
        '_op' : {},
        '_opchild' : [],
        
        'showWith' : function() {
            var argcount = arguments.length;
            if (argcount == 1) {
                this.set('visibility', arguments[0] ? 'visibile' : 'gone');
            } else if (argcount == 2) {
                if (typeof(arguments[1]) == 'string') {
                    this.set('visibility', arguments[0] ? 'visibile' : arguments[1]);
                } else {
                    if (arguments[0]) {
                        this.set('visibility', 'visibile');
                        for (attr in arguments[1]) {
                            this.set(attr, arguments[1][attr]);
                        }
                    } else {
                        this.set('visibility', 'gone');
                    }
                }
            } else if (argcount == 3) {
                if (arguments[0]) {
                    this.set('visibility', 'visibile');
                    for (attr in arguments[2]) {
                        this.set(attr, arguments[2][attr]);
                    }
                } else {
                    this.set('visibility', arguments[1]);
                }
            }
            return this;
        },
        
        'set' : function(attr, value) {
            this._op[attr] = value;
            return this;
        },
        
        'setChild' : function() {
            var item;
            var argcount = arguments.length;
            if (argcount > 0) { // 1st arg
                item = {};
                var action = arguments[0];
                if (action instanceof Array) {
                    item.oplist = action;
                } else {
                    item.oplist = [action.getAction()];
                }
                if (argcount > 1) { // 2nd arg
                    if (typeof(arguments[1]) == 'string') {
                        item.layout = arguments[1];
                    } else {
                        item.index = arguments[1];
                    }
                    if (argcount > 2) { // 3rd arg
                        item.index = arguments[2];
                    }
                }
            }
            this._opchild.push(item);
        },
        
        'apply' : function() {
            postAction(this.getAction());
            
            // clear old update actions;
            this._op = {};
            this._opchild = [];
        },
        
        'getAction' : function() {
            var action = {};
            if (this.id) action["id"] = this.id;
            if (this.tag) action["tag"] = this.tag;
            if (this._op) action["op"] = this._op;
            if (this._opchild && this._opchild.length > 0) action["opchild"] = this._opchild;
            return action;
        },
        
        'reset' : function() {
            this._op = {};
            this._opchild = [];
        }
    };
}

function update(attr, value) {
    var dict = {};
    dict[attr] = value;
    postAction({
        "op" : dict
    });
}

function byId(id) {
    var bean = createActionBean();
    bean["id"] = id;
    return bean;
}

function findViewById(id) {
    return byId(id);
}

function withTag(tag) {
    var bean = createActionBean();
    bean["tag"] = tag;
    return bean;
}

function findViewWithTag(tag) {
    withTag(tag);
}

function createBatch() {
    var argcount = arguments.length;
    if (argcount > 0) {
        var batch = createBatch();
        for (var i = 0; i < argcount; i++) {
            batch.append(arguments[i]);
        }
        return batch;
    }
    
    return {
        '_list':[],
        'append': function() {
            var count = arguments.length;
            if (count > 0) {
                for (var i = 0; i < count; i++) {
                    var task = arguments[i];
                    this._list.push(task.getAction());
                }
            }
            return this;
        },
        'getAction' : function() {
            return this._list;
        },
        'commit': function() {
            postAction(this._list);
            this._list = [];
        }
    };
}

function batch() {
    return createBatch();
}