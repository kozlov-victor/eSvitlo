import {AbstractTsxDOMRenderer} from "@engine/renderable/tsx/_genetic/abstractTsxDOMRenderer";
import {DomElementCreator, tDomElement} from "@engine/renderable/tsx/dom/internal/domElementCreator";


export class HtmlTsxDOMRenderer extends AbstractTsxDOMRenderer<tDomElement> {
    constructor() {
        super(new DomElementCreator());
    }
}
