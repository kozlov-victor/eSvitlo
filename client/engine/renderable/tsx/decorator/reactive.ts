import {VEngineTsxRootHolder} from "@engine/renderable/tsx/_genetic/vEngineTsxRootHolder";


export const Reactive = {
    Method: function() {
        return (originalMethod:any,context:ClassMethodDecoratorContext) => {
            context.addInitializer(function(){
                (this as any)[context.name] = ((...args: any[])=>{
                    const result = originalMethod.apply(this, args);
                    VEngineTsxRootHolder.ROOT?._triggerRendering();
                    if (result instanceof Promise) {
                        result.
                        then(()=>{
                            VEngineTsxRootHolder.ROOT?._triggerRendering();
                        }).
                        catch(()=>{
                            VEngineTsxRootHolder.ROOT?._triggerRendering();
                        });
                    }
                    return result;
                });
            });
        };
    },
    BoundedContext: function() {
        return (originalMethod:any,context:ClassMethodDecoratorContext) => {
            context.addInitializer(function(){
                (this as any)[context.name] = ((...args: any[])=>{
                    return originalMethod.apply(this, args);
                });
            });
        };
    },
    Property: function() {
        return (originalProperty:any,context:ClassFieldDecoratorContext) => {
            context.addInitializer(function(){
                let _val:any = undefined;
                Object.defineProperty(this, context.name,{
                    get: ()=>{
                        // Promise.resolve().then(()=>{
                        //     (this as VEngineTsxComponent)._triggerRendering();
                        // });
                        return _val;
                    },
                    set:val=>{
                        _val = val;
                        VEngineTsxRootHolder.ROOT?._triggerRendering();
                    }
                });
            });
        };
    },
    Function: function <K extends unknown[],U>(fn:(...args:[...K]) => U):(...args:[...K])=>U {
        return (...args: [...K]) => {
            const res = fn(...args);
            VEngineTsxRootHolder.ROOT._triggerRendering();
            if (res instanceof Promise) {
                res.
                then(()=>{
                    VEngineTsxRootHolder.ROOT?._triggerRendering();
                }).
                catch(()=>{
                    VEngineTsxRootHolder.ROOT?._triggerRendering();
                });
            }
            return res;
        };
    },
}
