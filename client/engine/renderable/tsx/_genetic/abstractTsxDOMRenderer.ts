import {VirtualCommentNode, VirtualNode} from "@engine/renderable/tsx/_genetic/virtualNode";
import {Optional} from "@engine/core/declarations";
import {AbstractElementCreator} from "@engine/renderable/tsx/_genetic/abstractElementCreator";
import {getComponentUuid, VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";
import {VirtualFragment} from "@engine/renderable/tsx/_genetic/virtualFragment";


export abstract class AbstractTsxDOMRenderer<T> {

    private oldVirtualDom:VirtualNode;
    private readonly allComponents: BaseTsxComponent[] = [];

    protected constructor(public readonly elementCreator:AbstractElementCreator<T>) {
    }

    public render(component:BaseTsxComponent, root:T):VirtualNode{
        const newVirtualNode = component.render() as VirtualNode|VirtualFragment;
        const newVirtualNodeChildren:VirtualNode[] = [];
        if (newVirtualNode.type==='virtualFragment') {
            newVirtualNodeChildren.push(...newVirtualNode.children);
        } else {
            newVirtualNodeChildren.push(newVirtualNode);
        }
        const newVirtualDom = new VirtualNode({},'root',newVirtualNodeChildren);
        this.reconcileChildren(newVirtualDom,this.oldVirtualDom,root);
        this.oldVirtualDom = newVirtualDom;

        for (const cmp of this.allComponents) {
            if (cmp.__shouldBeMounted) {
                cmp.__shouldBeMounted = false;
                cmp.onMounted();
            }
            cmp.onRendered();
        }
        this.allComponents.length = 0;

        return newVirtualDom;
    }

    private destroyNode(node:VirtualNode) {
        VEngineTsxFactory.destroyComponent(node);
        node.children.forEach(it=>this.destroyNode(it));
        node.parentComponent?.onDestroyed();
    }

    private replaceNode(node:T,oldVirtualNode:VirtualNode,newVirtualNode:VirtualNode,realParent:T,virtualParent:Optional<VirtualNode>) {
        const newNode = this.elementCreator.createElementByTagName(newVirtualNode);
        this.setGenericProps(newNode,newVirtualNode,undefined,realParent);
        this.elementCreator.replaceChild(realParent,node,newNode);
        const uuid = getComponentUuid(oldVirtualNode.props);
        if (virtualParent?.children.find(it=>getComponentUuid(it.props)===uuid)===undefined) {
            this.destroyNode(oldVirtualNode);
        }
        this.triggerOnRendered(newVirtualNode);
        return newNode;
    }

    private updateNode(node:T,newVirtualNode:VirtualNode,oldVirtualNode:VirtualNode,parent:T) {
        this.setGenericProps(node,newVirtualNode,oldVirtualNode,parent);
        this.triggerOnRendered(newVirtualNode);
    }

    private createNode(newVirtualNode:VirtualNode,parent:T) {
        const node = this.elementCreator.createElementByTagName(newVirtualNode);
        this.setGenericProps(node,newVirtualNode,undefined,parent);
        this.elementCreator.appendChild(parent,node);
        this.triggerOnRendered(newVirtualNode);
        return node;
    }

    private triggerOnRendered(newVirtualNode:VirtualNode) {
        if (newVirtualNode.parentComponent) {
            this.allComponents.push(newVirtualNode.parentComponent);
        }
    }

    private reconcile(
        newVirtualNode:Optional<VirtualNode>,
        oldVirtualNode:Optional<VirtualNode>,realNode:T,
        parent:T,
        virtualParent:Optional<VirtualNode>
    ) {

        //render node
        let newRealNode:Optional<T> = realNode;
        if (newVirtualNode===undefined && oldVirtualNode!==undefined) {  // remove node
            if (newRealNode!==undefined) {
                this.elementCreator.removeElement(newRealNode);
                this.destroyNode(oldVirtualNode);
            }
        }
        else if (newVirtualNode!==undefined && oldVirtualNode!==undefined && newRealNode!==undefined) {
            if (
                newVirtualNode.props?.trackBy !==oldVirtualNode.props?.trackBy ||
                newVirtualNode.props?.__id !==oldVirtualNode.props?.__id ||
                newVirtualNode.tagName!==oldVirtualNode.tagName
            ) { // replace node
                newRealNode = this.replaceNode(newRealNode,oldVirtualNode,newVirtualNode,parent,virtualParent);
            } else {
                this.updateNode(newRealNode,newVirtualNode,oldVirtualNode,parent); // update node
            }
        }
        else if (newVirtualNode!==undefined && (oldVirtualNode===undefined || newRealNode===undefined)){ // create new node
            newRealNode = this.createNode(newVirtualNode,parent);
        }
        // render children
        if (newRealNode!==undefined) this.reconcileChildren(newVirtualNode,oldVirtualNode,newRealNode);
        return newRealNode;
    }

    private reconcileChildren(newVirtualNode:Optional<VirtualNode>,oldVirtualNode:Optional<VirtualNode>,parent:T) {
        const maxNumOfChild =
            Math.max(
                newVirtualNode?.children?.length ?? 0,
                oldVirtualNode?.children?.length ?? 0
            );
        const realChildren:T[] = [];
        for (let i=0,max=this.elementCreator.getChildrenCount(parent);i<max;++i) {
            const ch = this.elementCreator.getChildAt(parent,i);
            realChildren.push(ch);
        }
        for (let i = 0;i<maxNumOfChild;++i) {
            const newVirtualChild = newVirtualNode?.children?.[i];
            const oldVirtualChild = oldVirtualNode?.children?.[i];
            this.reconcile(newVirtualChild,oldVirtualChild,realChildren[i] as T,parent,newVirtualNode);
        }
    }

    private setGenericProps(model:T,virtualNode:VirtualNode,oldVirtualNode:Optional<VirtualNode>,parent:T) {
        if (virtualNode?.props?.ref!==undefined) {
            if (virtualNode instanceof VirtualCommentNode) {
                virtualNode.props.ref(virtualNode.parentComponent);
            }
            else {
                virtualNode.props.ref(model);
            }
        }
        this.elementCreator.setProps(model,virtualNode,oldVirtualNode,parent);
    }

}
