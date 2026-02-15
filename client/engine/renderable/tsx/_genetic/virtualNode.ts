import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";

export interface IBaseProps {
    __id?:number;
    trackBy?:string;
    children?:any[];
    ref?:(el:any)=>void;
}

export class VirtualNode implements INode, JSX.Element {

    public type = 'virtualNode' as const;
    public parentComponent:BaseTsxComponent;
    public declare readonly __tsxElement__: 'Element';

    constructor(
        public readonly props: IBaseProps & Readonly<Record<string, any>>,
        public readonly tagName:string,
        public readonly children:VirtualNode[] = [],
    ) {}
}

export class VirtualTextNode extends VirtualNode {

    public text: string;

    constructor(text:string) {
        super({children: undefined!}, undefined!,undefined!);
        this.text = text;
    }
}

export class VirtualCommentNode extends VirtualNode {

    public comment: string;

    constructor(props: Readonly<Record<string, any>>, comment:string) {
        super(props, undefined!,undefined!);
        this.comment = comment;
    }
}
