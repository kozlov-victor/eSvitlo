import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {Dialog} from "../dialog/dialog";
import {IBaseProps} from "@engine/renderable/tsx/_genetic/virtualNode";

export class PromptDialog extends Dialog<{title?:string, text: string|JSX.Element, buttons?:[string,string]},boolean> {

    constructor(private props: IBaseProps & {title?:string, text: string|JSX.Element, buttons?:[string,string]}) {
        super(props);
    }

    protected override renderDialog(): JSX.Element {
        return (
            <>
                <div className={'dialog-text-content'}>{this.props.text}</div>
                <div>
                    <button onclick={_=>this.close(true)}>{this.props?.buttons?.[0] || 'Так'}</button>
                    <button onclick={_=>this.close(false)}>{this.props?.buttons?.[1] || 'Ні'}</button>
                </div>
            </>
        );
    }
}
