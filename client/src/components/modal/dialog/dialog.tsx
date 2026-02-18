import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {IBaseProps} from "@engine/renderable/tsx/_genetic/virtualNode";
import {DI} from "@engine/core/ioc";
import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";

export class DialogRef {
    private onClosed:(result:any)=>void;
    public _elFactory: ()=>JSX.Element;
    public _clazz: {new(props:any): any}

    @Reactive.Method()
    public close(result?: any): void {
        this.onClosed?.(result);
    }

    public waitForResult<T>() {
        return new Promise<T>(resolve => {
            this.onClosed = resolve;
        });
    }
}

@DI.Injectable()
@DI.CSS('./dialog.css')
export class BaseDialogService {

    private readonly refs:DialogRef[] = [];
    private cnt = 0;

    private normalizeProps(props?:any) {
        if (!props) props = {};
        props.__id = `dialog_${this.cnt++}_${props?.__id ?? ''}`;
        return props;
    }

    @Reactive.Method()
    public async open<TOptions extends object,TResult>(clazz:new(props:any) => Dialog<TOptions, TResult>, props?: TOptions):Promise<TResult> {
        const ref = new DialogRef();
        props = this.normalizeProps(props);
        ref._elFactory = ()=>VEngineTsxFactory.createElement(clazz as any,props as any);
        ref._clazz = clazz;
        this.refs.push(ref);
        return ref.waitForResult<TResult>();
    }

    @Reactive.Method()
    public openSync<TOptions extends object,TResult>(clazz:new(props:any) => Dialog<TOptions, TResult>, props?: TOptions):DialogRef {
        const ref = new DialogRef();
        props = this.normalizeProps(props);
        ref._elFactory = ()=>VEngineTsxFactory.createElement(clazz as any,props as any);
        ref._clazz = clazz;
        this.refs.push(ref);
        const refCloseBase = ref.close.bind(ref);
        ref.close = ()=>{
            this.refs.splice(this.refs.indexOf(ref),1);
            Reactive.Function(()=>refCloseBase())();
        }
        return ref;
    }

    @Reactive.Method()
    public close<TOptions extends object, TResult>(instance:Dialog<TOptions, TResult>,result:TResult) {
        const ref = this.refs.find(it=>instance instanceof (it._clazz as any));
        if (!ref) {
            console.error(instance);
            throw new Error('wrong dialog ref');
        }
        this.refs.splice(this.refs.indexOf(ref),1);
        ref.close(result);
    }

    public getOutlet() {
        return <>{this.refs.map(it=>it._elFactory())}</>
    }

}

let cnt = 0;

@DI.Injectable()
export class Dialog<TOptions extends object, TResult> extends BaseTsxComponent {

    @DI.Inject(BaseDialogService) protected service: BaseDialogService;

    constructor(private prps: IBaseProps & {closeable?:false} & TOptions) {
        super();
    }

    protected renderDialog():JSX.Element {
        return <></>
    }

    protected getTitle() {
        return `Увага!`;
    }

    protected close(result?: TResult) {
        this.service.close(this,result);
    }

    render(): JSX.Element {
        return (
            <div className="modal-wrap-outer">
                <div className="modal-wrap-inner">
                    <div className="dialog">
                        <div className="dialog-head">
                            <h4 className={'dialog-title'}>{this.getTitle()}</h4>
                            {
                                this.prps.closeable!==false &&
                                <span className="dialog-close" onclick={_ => this.close(undefined)}>✖</span>
                            }
                        </div>
                        <div className="dialog-body">
                            {this.renderDialog()}
                        </div>
                    </div>
                </div>
            </div>
        );
    }
}
