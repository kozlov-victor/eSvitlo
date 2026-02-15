import {VirtualNode} from "@engine/renderable/tsx/_genetic/virtualNode";
import {Optional} from "@engine/core/declarations";

export abstract class AbstractElementCreator<T> {

    public abstract createElementByTagName(node:VirtualNode): T;
    public abstract setProps(model:T,virtualNode:VirtualNode,oldVirtualNode:Optional<VirtualNode>,parent:T):void;

    public abstract appendChild(el:T,child: T): void;

    public abstract removeElement(el:T): void;

    public abstract replaceChild(el:T,oldNode: T, newNode: T): void;

    public abstract getChildAt(el:T,index:number):T;

    public abstract getChildrenCount(el:T): number;

    public abstract getParentNode(el:T): T;

    public abstract insertChildAt(el:T,node: T, i: number): void;

    public abstract removeChildren(el:T): void;

}
