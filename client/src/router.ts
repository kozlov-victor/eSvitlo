import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {DI} from "@engine/core/ioc";

interface IElementFactoryItem {
    component: JSX.Element;
    data?: any;
}



@DI.Injectable()
export class Router {

    private readonly registry:Record<string, ()=>IElementFactoryItem> = {};

    private readonly onNavigated = Reactive.Function(()=>{});

    constructor() {
        window.addEventListener('hashchange',e=>{
            this.onNavigated();
        });
    }

    public setUp(map:Record<string, ()=>IElementFactoryItem>) {
        Object.keys(map).forEach((key:string) => {
            this.registry[key] = map[key];
        });
    }

    public getFactoryItemByRoute(){
        let elementFactory:()=>IElementFactoryItem = undefined!;
        const path = this.getPath();
        if (this.registry[path]) elementFactory = this.registry[path];
        else if (path==='' && this.registry['/']) elementFactory = this.registry['/'];
        else if (this.registry['*']) elementFactory = this.registry['*'];
        else throw new Error(`no component found for ${path}`);
        return elementFactory();
    }

    public getOutlet(){
        return this.getFactoryItemByRoute().component;
    }

    public getDataByRoute(){
        return this.getFactoryItemByRoute().data;
    }

    public navigate(path: string, params:Record<string, string|number>= {}){
        let urlParams:string[] = undefined!;
        Object.keys(params).forEach(k=>{
            const val = params[k]===null || params[k]===undefined?'':encodeURIComponent(params[k]);
            if (urlParams===undefined) urlParams = [];
            urlParams.push(`${k}=${val}`);
        });
        location.hash = `${path}${urlParams===undefined?'':`?${urlParams.join('&')}`}`;
    }

    public getPath() {
        let path = location.hash.replace('#','');
        path = path.split('?')[0];
        if (path==='/') path = '';
        else if (path.startsWith('/')) {
            path = path.substr(1);
        }
        return path;
    }

    public getUrlParams() {
        const path = location.hash.replace('#','');
        const paramsStr = path.split('?')[1];
        if (!paramsStr) return {};
        const params:Record<string, string|undefined> = {};
        paramsStr.split('&').forEach(pair=>{
            const kv = pair.split('=');
            const k = kv[0];
            let v: string|undefined = undefined;
            if(kv[1]!==undefined) {
                v = decodeURIComponent(kv[1]);
            }
            params[k] = v;
        });
        return params;
    }

}
