const ts = require("typescript");


const globalStateSymbol = Symbol.for("tsxPrecompilerGlobalState");

module.exports = function (content) {

    if (!this.resourcePath?.endsWith('.tsx')) return content;

    const compilation = this._compilation;
    if (!compilation[globalStateSymbol]) {
        compilation[globalStateSymbol] = {
            cnt: 0
        };
    }

    const sourceFile = ts.createSourceFile(
        "temp.tsx",
        content,
        ts.ScriptTarget.Latest,
        true,
        ts.ScriptKind.TSX
    );

    const transformer = context => {
        const visit = node => {
            if (ts.isJsxOpeningElement(node) || ts.isJsxSelfClosingElement(node)) {
                const hasId = node.attributes.properties.some(prop => ts.isJsxAttribute(prop) && prop.name.text === "__id");
                if (!hasId) {
                    const newAttr = ts.factory.createJsxAttribute(
                        ts.factory.createIdentifier("__id"),
                        ts.factory.createJsxExpression(undefined, ts.factory.createNumericLiteral(compilation[globalStateSymbol].cnt))
                    );
                    compilation[globalStateSymbol].cnt++;
                    const newProps = ts.factory.createNodeArray([
                        ...node.attributes.properties,
                        newAttr
                    ]);
                    const updatedAttrs = ts.factory.updateJsxAttributes(node.attributes, newProps);
                    if (ts.isJsxOpeningElement(node)) {
                        return ts.factory.updateJsxOpeningElement(
                            node,
                            node.tagName,
                            node.typeArguments,
                            updatedAttrs
                        );
                    } else {
                        return ts.factory.updateJsxSelfClosingElement(
                            node,
                            node.tagName,
                            node.typeArguments,
                            updatedAttrs
                        );
                    }
                }
            }
            return ts.visitEachChild(node, visit, context);
        };
        return node => ts.visitNode(node, visit);
    };

    const result = ts.transform(sourceFile, [transformer]);
    const transformed = result.transformed[0];
    const printer = ts.createPrinter();

    const transformedSource = printer.printFile(transformed)
    //console.log(transformedSource);
    return transformedSource;
};
