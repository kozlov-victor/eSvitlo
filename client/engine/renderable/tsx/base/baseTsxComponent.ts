import {VEngineTsxRootHolder} from "@engine/renderable/tsx/_genetic/vEngineTsxRootHolder";
import {IBaseProps} from "@engine/renderable/tsx/_genetic/virtualNode";


export abstract class BaseTsxComponent implements JSX.Element {

    public static readonly __VEngineTsxComponent = true as const;
    public __shouldBeMounted = true;
    public declare readonly __tsxElement__: 'Element';

    public abstract render():JSX.Element;
    public onMounted() {}
    public onRendered() {}

    public onDestroyed() {}

    public onPropsReceived(props: IBaseProps & Record<string, any>) {}

    public _triggerRendering():void{
        VEngineTsxRootHolder.ROOT._triggerRendering();
    }

}
