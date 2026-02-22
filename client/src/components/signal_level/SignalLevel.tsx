import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";
import {DI} from "@engine/core/ioc";
import {IBaseProps} from "@engine/renderable/tsx/_genetic/virtualNode";


@DI.CSS('./SignalLevel.css')
export class SignalLevel extends BaseTsxComponent {

    private readonly bars = [
        {base: -100},
        {base: -95},
        {base: -90},
        {base: -80},
        {base: -70},
    ]

    constructor(private props:IBaseProps & {level: number}) {
        super();
    }

    private getBarHeight(i:number) {
        let h = 12 * (i+1)/this.bars.length;
        if (h===0) h = 1;
        return h;
    }

    render(): JSX.Element {
        const l= this.props.level;
        return  (
            <div className={'signal-level'}>
                {this.bars.map((bar,i)=>
                    <div
                        style={{height:`${this.getBarHeight(i)}px`}}
                        className={'bar'}
                        classNames={{'bar-empty':l<bar.base,'bar-filled':l>=bar.base}}/>
                )}
                <div className={'level-text'}>{l} dB</div>
            </div>
        );
    }

}