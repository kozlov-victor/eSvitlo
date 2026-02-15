
interface ICommonElement<T> {
    className?: string;
    classNames?:Record<string, boolean>;
    key?:number|string;
    ref?:(el:T)=>void;
    __id?: number;
    style?:Partial<CSSStyleDeclaration>;
}

type TCommonElement<T> = ICommonElement<T> & Partial<Omit<T,'style'|'click'>>;

interface INode {
    tagName:string;
    children:INode[];
    type: 'virtualNode'|'virtualFragment';
}

declare namespace JSX {
    // tslint:disable-next-line:interface-name
    export interface IntrinsicElements {
        a: TCommonElement<HTMLAnchorElement>;
        abbr: TCommonElement<HTMLElement>;
        address: TCommonElement<HTMLElement>;
        area: TCommonElement<HTMLAreaElement>;
        article: TCommonElement<HTMLAreaElement>;
        aside: TCommonElement<HTMLElement>;
        audio: TCommonElement<HTMLAudioElement>;
        b: TCommonElement<HTMLElement>;
        base: TCommonElement<HTMLBaseElement>;
        bdi: TCommonElement<HTMLElement>;
        bdo: TCommonElement<HTMLElement>;
        big: TCommonElement<HTMLElement>;
        blockquote: TCommonElement<HTMLElement>;
        body: TCommonElement<HTMLBodyElement>;
        br: TCommonElement<HTMLBRElement>;
        button: TCommonElement<HTMLButtonElement>;
        canvas: TCommonElement<HTMLCanvasElement>;
        caption: TCommonElement<HTMLElement>;
        cite: TCommonElement<HTMLElement>;
        code: TCommonElement<HTMLElement>;
        col: TCommonElement<HTMLTableColElement>;
        colgroup: TCommonElement<HTMLTableColElement>;
        data: TCommonElement<HTMLDataElement>;
        datalist: TCommonElement<HTMLDataListElement>;
        dd: TCommonElement<HTMLElement>;
        del: TCommonElement<HTMLElement>;
        details: TCommonElement<HTMLElement>;
        dfn: TCommonElement<HTMLElement>;
        dialog: TCommonElement<HTMLDialogElement>;
        div: TCommonElement<HTMLDivElement>;
        dl: TCommonElement<HTMLDListElement>;
        dt: TCommonElement<HTMLElement>;
        em: TCommonElement<HTMLElement>;
        embed: TCommonElement<HTMLEmbedElement>;
        fieldset: TCommonElement<HTMLFieldSetElement>;
        figcaption: TCommonElement<HTMLElement>;
        figure: TCommonElement<HTMLElement>;
        footer: TCommonElement<HTMLElement>;
        form: TCommonElement<HTMLFormElement>;
        h1: TCommonElement<HTMLHeadingElement>;
        h2: TCommonElement<HTMLHeadingElement>;
        h3: TCommonElement<HTMLHeadingElement>;
        h4: TCommonElement<HTMLHeadingElement>;
        h5: TCommonElement<HTMLHeadingElement>;
        h6: TCommonElement<HTMLHeadingElement>;
        head: TCommonElement<HTMLHeadElement>;
        header: TCommonElement<HTMLElement>;
        hgroup: TCommonElement<HTMLElement>;
        hr: TCommonElement<HTMLHRElement>;
        html: TCommonElement<HTMLHtmlElement>;
        i: TCommonElement<HTMLElement>;
        iframe: TCommonElement<HTMLIFrameElement>;
        img: TCommonElement<HTMLImageElement>;
        input: ICommonElement<HTMLInputElement> &
            Partial<Omit<HTMLInputElement, 'style'|'onchange'|'oninput'|'click'>> &
            {
                onchange?: (e: Event & { target: HTMLInputElement }) => void,
                oninput?: (e: Event & { target: HTMLInputElement }) => void,
            };
        ins: TCommonElement<HTMLModElement>;
        kbd: TCommonElement<HTMLElement>;
        keygen: TCommonElement<HTMLElement>;
        // eslint-disable-next-line @typescript-eslint/ban-types
        label: TCommonElement<HTMLLabelElement> & Partial<Omit<HTMLLabelElement, "style"> & { htmlFor?: string | null }>;
        legend: TCommonElement<HTMLLegendElement>;
        li: TCommonElement<HTMLLIElement>;
        link: TCommonElement<HTMLLinkElement>;
        main: TCommonElement<HTMLElement>;
        map: TCommonElement<HTMLMapElement>;
        mark: TCommonElement<HTMLElement>;
        menu: TCommonElement<HTMLElement>;
        menuitem: TCommonElement<HTMLElement>;
        meta: TCommonElement<HTMLMetaElement>;
        meter: TCommonElement<HTMLElement>;
        nav: TCommonElement<HTMLElement>;
        noindex: TCommonElement<HTMLElement>;
        noscript: TCommonElement<HTMLElement>;
        object: TCommonElement<HTMLObjectElement>;
        ol: TCommonElement<HTMLOListElement>;
        optgroup: TCommonElement<HTMLOptGroupElement>;
        option: TCommonElement<HTMLOptionElement>;
        output: TCommonElement<HTMLElement>;
        p: TCommonElement<HTMLParagraphElement>;
        param: TCommonElement<HTMLParamElement>;
        picture: TCommonElement<HTMLElement>;
        pre: TCommonElement<HTMLPreElement>;
        progress: TCommonElement<HTMLProgressElement>;
        q: TCommonElement<HTMLQuoteElement>;
        rp: TCommonElement<HTMLElement>;
        rt: TCommonElement<HTMLElement>;
        ruby: TCommonElement<HTMLElement>;
        s: TCommonElement<HTMLElement>;
        samp: TCommonElement<HTMLElement>;
        slot: TCommonElement<HTMLSlotElement>;
        script: TCommonElement<HTMLScriptElement>;
        section: TCommonElement<HTMLElement>;
        select:
            ICommonElement<HTMLSelectElement> &
            Partial<Omit<HTMLSelectElement, 'style' | 'onchange'>> &
            { onchange: (e: Event & { target: HTMLSelectElement }) => void };
        small: TCommonElement<HTMLElement>;
        source: TCommonElement<HTMLSourceElement>;
        span: TCommonElement<HTMLSpanElement>;
        strong: TCommonElement<HTMLElement>;
        style: TCommonElement<HTMLElement>;
        sub: TCommonElement<HTMLElement>;
        summary: TCommonElement<HTMLElement>;
        sup: TCommonElement<HTMLElement>;
        table: TCommonElement<HTMLTableElement>;
        template: TCommonElement<HTMLTemplateElement>;
        tbody: TCommonElement<HTMLTableSectionElement>;
        td: TCommonElement<HTMLTableCellElement>;
        textarea: TCommonElement<HTMLTextAreaElement>;
        tfoot: TCommonElement<HTMLTableSectionElement>;
        th: TCommonElement<HTMLTableCellElement>;
        thead: TCommonElement<HTMLTableSectionElement>;
        time: TCommonElement<HTMLElement>;
        tr: TCommonElement<HTMLTableRowElement>;
        track: TCommonElement<HTMLTrackElement>;
        u: TCommonElement<HTMLElement>;
        ul: TCommonElement<HTMLUListElement>;
        var: TCommonElement<HTMLElement>;
        video: TCommonElement<HTMLVideoElement>;
        wbr: TCommonElement<HTMLElement>;
        font: TCommonElement<HTMLFontElement>;

        svg: any;
        g: any;
        defs: any;
        desc: any;
        title: any;
        symbol: any;
        use: any;

        path: any;
        rect: any;
        circle: any;
        ellipse: any;
        line: any;
        polyline: any;
        polygon: any;

        text: any;
        tspan: any;
        tref: any;
        textPath: any;

        image: any;

        marker: any;
        pattern: any;
        clipPath: any;
        mask: any;

        linearGradient: any;
        radialGradient: any;
        stop: any;

        filter: any;
        feBlend: any;
        feColorMatrix: any;
        feComponentTransfer: any;
        feComposite: any;
        feConvolveMatrix: any;
        feDiffuseLighting: any;
        feDisplacementMap: any;
        feDistantLight: any;
        feDropShadow: any;
        feFlood: any;
        feFuncA: any;
        feFuncB: any;
        feFuncG: any;
        feFuncR: any;
        feGaussianBlur: any;
        feImage: any;
        feMerge: any;
        feMergeNode: any;
        feMorphology: any;
        feOffset: any;
        fePointLight: any;
        feSpecularLighting: any;
        feSpotLight: any;
        feTile: any;
        feTurbulence: any;
        animate: any;
        set: any;
        animateMotion: any;
        mpath: any;
        animateTransform: any;
    }

    export interface Element {
        __tsxElement__:'Element';
    }
}
