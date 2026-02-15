import {AbstractElementCreator} from "@engine/renderable/tsx/_genetic/abstractElementCreator";
import {VirtualCommentNode, VirtualNode, VirtualTextNode} from "@engine/renderable/tsx/_genetic/virtualNode";
import {Optional} from "@engine/core/declarations";

const ELEMENT_PROPERTIES: Readonly<string[]> = ['value','checked','selected','focus','disabled','readonly','innerHTML','textContent'];
const SPECIAL_ATTRIBUTES: Readonly<string[]> = ['children','__id'];

const svgTags: Readonly<string[]> = [
    'svg', 'g', 'defs', 'desc', 'title', 'symbol', 'use',
    'path', 'rect', 'circle', 'ellipse', 'line', 'polyline', 'polygon',
    'text', 'tspan', 'tref', 'textPath',
    'image',
    'marker', 'pattern', 'clipPath', 'mask',
    'linearGradient', 'radialGradient', 'stop',
    'filter', 'feBlend', 'feColorMatrix', 'feComponentTransfer', 'feComposite',
    'feConvolveMatrix', 'feDiffuseLighting', 'feDisplacementMap', 'feDistantLight',
    'feDropShadow', 'feFlood', 'feFuncA', 'feFuncB', 'feFuncG', 'feFuncR',
    'feGaussianBlur', 'feImage', 'feMerge', 'feMergeNode', 'feMorphology',
    'feOffset', 'fePointLight', 'feSpecularLighting', 'feSpotLight',
    'feTile', 'feTurbulence',
    'style', 'script', 'animate', 'set', 'animateMotion', 'mpath',
    'animateTransform'
] as const;
const objEq = (a:Record<any, any>, b: Record<any, any>):boolean=>{
    if (a===b) return true;
    if (!a || !b) return false;
    const kA = Object.keys(a);
    const kB = Object.keys(b);
    for (const ka of kA) if (kB.indexOf(ka)===-1) return false;
    for (const k of kA) if ((a[k]!==b[k])) return false;
    return true;
}

export type tDomElement = Text|Comment|HTMLElement|SVGElement;

export class DomElementCreator extends AbstractElementCreator<tDomElement>{

    constructor() {
        super();
    }

    createElementByTagName(node:VirtualNode): tDomElement {
        let htmlNode:Text|HTMLElement|Comment|SVGElement;
        if (node instanceof VirtualTextNode) {
            htmlNode = document.createTextNode(node.text);
        }
        else if (node instanceof VirtualCommentNode) {
            htmlNode = document.createComment(node.comment);
        }
        else {
            if (svgTags.indexOf(node.tagName)>-1) {
                htmlNode = document.createElementNS('http://www.w3.org/2000/svg',node.tagName);
            }
            else htmlNode = document.createElement(node.tagName);
        }
        return htmlNode;
    }

    setProps(el: tDomElement, virtualNode:VirtualNode,oldVirtualNode:Optional<VirtualNode>,parent:tDomElement): void {
        const props = virtualNode.props;
        if (el.nodeType==8) { // comment node
            return;
        }
        if (el.nodeType===3) {
            const virtualTextNode = virtualNode as VirtualTextNode;
            const oldVirtualTextNode = oldVirtualNode as Optional<VirtualTextNode>;
            if (!oldVirtualTextNode || virtualTextNode.text!==oldVirtualTextNode.text) {
                (el as Text).data = virtualTextNode.text;
            }
        } else {

            if (!props) {
                console.error(`something is wrong with this node`);
                console.error({virtualNode, parent});
                return;
            }

            const htmlEl = el as HTMLElement;
            for (const key of Object.keys(props)) {
                if (key.indexOf('on')===0) {// events
                    (htmlEl as Record<string, any>)[key] = props[key];
                }
                else if (key==='dataset') {
                    const dataset = props[key] ?? {};
                    for (const dataKey of Object.keys(dataset)) { // todo reconcile old object with new one
                        htmlEl.dataset[dataKey] = dataset[dataKey];
                    }
                }
                else if (!oldVirtualNode || oldVirtualNode.props[key]!==props[key]) {
                    let attrName = key;
                    if (SPECIAL_ATTRIBUTES.indexOf(attrName)>-1) continue;
                    if (attrName==='style') {
                        if (objEq(props[key],oldVirtualNode?.props[key])) continue;
                        const styleDeclarationNew = props[key];
                        const styleDeclarationOld = virtualNode.props.style;
                        Object.keys(styleDeclarationNew).forEach(k=>(htmlEl.style as any)[k]=styleDeclarationNew[k]);
                        Object.keys(styleDeclarationOld).forEach(k=>{
                            if (styleDeclarationNew[k]===undefined) (htmlEl.style as any)[k]=undefined;
                        });
                        continue;
                    }

                    let val = props[key];

                    if (key==='ref') {
                        props.ref!(el);
                        continue;
                    }
                    else if (key==='htmlFor') attrName = 'for';
                    else if (key==='className') {
                        if (props['classNames']) continue;
                        attrName = 'class';
                    }
                    else if (key==='classNames') {
                        const className = Object.keys(props[key]).filter(it=>props[key][it]).join(' ');
                        const oldClassName =
                            !oldVirtualNode?undefined:
                            Object.keys(oldVirtualNode.props[key]).filter(it=>oldVirtualNode.props[key][it]).join(' ');
                        if (className===oldClassName) continue;
                        attrName = 'class';
                        val = className;
                        if (props['className']) val+=' ' + props['className'];
                    }

                    if (ELEMENT_PROPERTIES.indexOf(key)>-1) { // property
                        (htmlEl as any)[key] = val ?? '';
                    }
                    else { // attribute
                        const value = val;
                        if (value===null || value===undefined) htmlEl.removeAttribute(attrName);
                        else htmlEl.setAttribute(attrName,val);
                    }

                }
            }

        }
    }


    public appendChild(el:tDomElement, child: tDomElement): void {
        if ((el as any).styleSheet && !(el as any).sheet) { // ie8
            (el as any).styleSheet.cssText=(child as any).data;
        }
        else el.appendChild(child);
    }

    public removeElement(el:tDomElement): void {
        el.parentNode!.removeChild(el);
    }

    public replaceChild(el:tDomElement,oldNode: tDomElement, newNode: tDomElement): void {
        el.replaceChild(newNode,oldNode);
    }

    public getChildAt(el:tDomElement,index:number):tDomElement{
        if (el.nodeType===3) return undefined!;
        return el.childNodes[index] as tDomElement;
    }

    public getChildrenCount(el:tDomElement): number {
        if (el.nodeType===3) return 0;
        return (el as HTMLElement).childNodes.length;
    }

    public getParentNode(el:tDomElement): tDomElement {
        return el.parentElement as tDomElement;
    }

    insertChildAt(el:tDomElement, node: tDomElement, i: number): void {
        const children = el.childNodes;
        const child = node;
        if (i >= children.length) {
            el.appendChild(child);
        } else {
            el.insertBefore(child, children[i]);
        }
    }

    public removeChildren(el:tDomElement): void {
        if (el.nodeType===3) {
            (el as Text).data = '';
        }
        (el as HTMLElement).innerHTML = '';
    }


}
