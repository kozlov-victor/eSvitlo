import {VirtualNode} from "@engine/renderable/tsx/_genetic/virtualNode";
import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";

export class VirtualFragment implements INode,JSX.Element{

    public tagName = undefined!;
    public type = 'virtualFragment' as const;
    public declare readonly __tsxElement__: 'Element';
    public parentComponent:BaseTsxComponent;

    constructor(public readonly children:VirtualNode[] = []) {}

}
