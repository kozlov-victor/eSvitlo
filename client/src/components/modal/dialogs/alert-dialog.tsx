import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {IBaseProps} from "@engine/renderable/tsx/_genetic/virtualNode";
import {Dialog} from "../dialog/dialog";

export class AlertDialog extends Dialog<{text: string|JSX.Element,title?:string,closeable?:false},void> {

    constructor(private props: IBaseProps & {text: string,closeable?:false}) {
        super(props);
    }


    protected override renderDialog(): JSX.Element {
        return (
            <>
                <div className={'dialog-text-content'}>{this.props.text}</div>
                {
                    this.props.closeable!==false &&
                    <div>
                        <button onclick={_=>this.close(undefined)}>Ok</button>
                    </div>
                }
            </>
        )
    }
}
