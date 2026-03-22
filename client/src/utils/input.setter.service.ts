import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {DI} from "@engine/core/ioc";

export type tInputEvent =
    Event & {target: HTMLInputElement|HTMLSelectElement|HTMLTextAreaElement};

@DI.Injectable()
export class InputSetterService {

    @Reactive.Method()
    public setValue<K extends keyof U,U>(e: tInputEvent, model: U,key: K, parser: (raw: string) => U[K]) {
        if ((parser as any)===Boolean) {
            model[key] = (e.target  as any).checked
        }
        else {
            model[key] = parser(e.target.value);
        }
    }

    public bind<K extends keyof U,U>(model: U,key: K, parser: (raw: string) => U[K]) {
        if ((parser as any)===Boolean) {
            return {
                checked: model[key] as boolean,
                onchange: (e: tInputEvent)=>this.setValue(e,model,key,parser),
            }
        }
        else {
            return {
                value: `${model[key] ?? ''}`,
                onchange: (e: tInputEvent)=>this.setValue(e,model,key,parser),
            }
        }
    }

}

export const Numeric = (val:string)=>{
    if (!val) return undefined;
    return +val;
}