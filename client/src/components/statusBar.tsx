import {IBaseProps} from "@engine/renderable/tsx/_genetic/virtualNode";
import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";

export const StatusBar = (props:IBaseProps & {text?:string,success?:boolean})=> {
    return (
        <div className={`status_bar ${props.success?'ok':'error'}`}>
            {props.text || 'Ok'}
        </div>
    );
}
