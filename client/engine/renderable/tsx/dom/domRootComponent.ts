
import {HtmlTsxDOMRenderer} from "@engine/renderable/tsx/dom/internal/htmlTsxDOMRenderer";
import {RootTsxComponent} from "@engine/renderable/tsx/base/rootTsxComponent";
import {tDomElement} from "@engine/renderable/tsx/dom/internal/domElementCreator";

export abstract class DomRootComponent extends RootTsxComponent<tDomElement> {
    constructor() {
        super(new HtmlTsxDOMRenderer());
    }
}
