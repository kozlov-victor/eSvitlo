import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {IBaseProps} from "@engine/renderable/tsx/_genetic/virtualNode";


export const Frame = (props:IBaseProps & {children?:any,title?:string})=> {
    return (
        <>
            <div className={'frame_out'}>
                <div className={'frame_title_wrap'}>
                    <div className={'frame_title'}>{props.title}</div>
                </div>
                <div className={'frame_in_1'}>
                    <div className={'frame_in_2'}>
                        {props.children}
                    </div>
                </div>
            </div>
        </>
    );
}
