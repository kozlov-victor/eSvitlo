

declare const setTimeout:(f:()=>void,t:number)=>number;

interface IKeyVal<T> {
    [key:string]:T;
}

const noop = (...args:any[])=>{};
const CONTENT_TYPE = "Content-Type";

const objectToQuery = (o:string|number|FormData|IKeyVal<string|number|boolean>):string|FormData=> {
    if (o===undefined || o===null) return '';
    if (o instanceof FormData) return o;
    const paramsArr:([string,string|number|boolean])[] = [];
    if (typeof o==='string' || typeof o==='number')
        return o.toString();
    Object.keys(o).forEach(key=>{
        paramsArr.push([key,encodeURIComponent(o[key])]);
    });
    return paramsArr.map((item)=>[item[0]+'='+item[1]]).join('&');
};

interface IRequestData<T> {
    method: string;
    data?: string|number|FormData|IKeyVal<string|number|boolean>;
    url: string;
    options: IRequestOptions<T>;
}

export interface IRequestOptions<T> {
    headers?: Record<string, string>;
    timeout?: number;
    ontimeout?: ()=>void;
    onProgress?:(e:SseEvent<T>)=>void;
}

interface SseEvent<T> {
    event: string;
    data:T;
}

function parseSSEEvent<T>(raw: string):SseEvent<T> {
    const lines = raw.split("\n");

    const event: SseEvent<T> = {
        event: 'message',
        data: undefined!,
    };

    let evt = '';

    lines.forEach(line => {
        if (line.startsWith('event:')) {
            evt = line.slice(6).trim();
        }
        else if (line.startsWith('data:')) {
            evt += line.slice(5).trim();
        }
    });

    try {
        event.data = JSON.parse(evt);
    } catch {}

    return event;
}

export namespace HttpClient {

    const request = <T>(data:IRequestData<T>):Promise<T>=> {
        let abortTmr:number;
        let resolved = false;
        data.method = data.method || 'get';
        if (data.data && data.method==='get') data.url+='?'+objectToQuery(data.data);
        const xhr=new XMLHttpRequest();
        let resolveFn = noop, rejectFn = noop;
        const promise = new Promise<T>((resolve,reject)=>{
            resolveFn = resolve;
            rejectFn = reject;
        });
        let lastIndex = 0;
        let buffer = '';

        function processChunk():SseEvent<T>|undefined {
            const chunk = xhr.responseText.substring(lastIndex);
            lastIndex = xhr.responseText.length;

            if (!chunk) return;

            buffer += chunk;

            let boundary;
            while ((boundary = buffer.indexOf("\n\n")) !== -1) {
                const rawEvent = buffer.substring(0, boundary);
                buffer = buffer.substring(boundary + 2);
                return parseSSEEvent<T>(rawEvent);
            }
            return undefined;
        }

        xhr.onreadystatechange=()=> {
            if (xhr.readyState === 3) {
                if (data.options.onProgress) {
                    const ev = processChunk();
                    if (ev) data.options.onProgress(ev);
                }
            }
            else if (xhr.readyState===4) {
                if ( (xhr.status>=200 && xhr.status<=299)) {
                    if (data.options.onProgress) {
                        // якщо в buffer ще щось лишилось (без \n\n)
                        let ev = processChunk();
                        if (ev) data.options.onProgress(ev);
                        if (buffer.trim().length > 0) {
                            ev = parseSSEEvent<T>(buffer);
                            data.options.onProgress(ev);
                            buffer = '';
                        }
                        resolveFn(ev?.data);
                    }
                    else {
                        let resp = xhr.responseText;
                        const contentType = xhr.getResponseHeader(CONTENT_TYPE)||'';
                        if (contentType.toLowerCase().indexOf('/json')>-1 && resp && resp.substr!==undefined) {
                            resp = JSON.parse(resp);
                        }
                        resolveFn(resp as unknown as T);
                    }
                } else {
                    let response:any;
                    try{
                        if (xhr.response) response = JSON.parse(xhr.response);
                    } catch (e) {
                        response = xhr.response;
                    }
                    rejectFn({status:xhr.status,error:xhr.statusText,response});
                }
                clearTimeout(abortTmr);
                resolved = true;
            }
        };
        xhr.onerror = e=>{
            rejectFn({status:xhr.status,error:xhr.statusText,response: e});
        };
        xhr.open(data.method,data.url,true);
        if (data.options?.headers) {
            Object.keys(data.options.headers).forEach(key=>{
                xhr.setRequestHeader(key,data.options.headers![key]);
            });
        }
        if (data.options.headers?.[CONTENT_TYPE]==='application/json') {
            data.data = data.data && JSON.stringify(data.data);
        }
        xhr.send(data.data as unknown as string);
        if (data.options.timeout) {
            abortTmr = setTimeout(()=>{
                if (resolved) return;
                xhr.abort();
                data.options.ontimeout?.();
                rejectFn({status:xhr.status,error:xhr.statusText,response: 'timeout'});
            },data.options.timeout);
        }
        return promise as Promise<T>;
    };

    export const get = <T>(url:string,data?:IKeyVal<string|number|boolean>,options:IRequestOptions<T> = {}):Promise<T>=>{
        options.headers??={};
        options.headers[CONTENT_TYPE]??='application/x-www-form-urlencoded';
        return request<T>({
            method:'get',
            url,
            data,
            options,
        });
    };

    export const  post = <T>(url:string,data?:any,options:IRequestOptions<T> = {}):Promise<T>=>{
        options.headers??={};
        options.headers[CONTENT_TYPE]??='application/json';
        return request<T>({
            method:'post',
            url,
            data,
            options,
        });
    };

    export const postMultiPart = <T>(url:string,file:File,data:IKeyVal<string|number|boolean>,options:IRequestOptions<T> = {}):Promise<T>=>{
        options.headers??={};
        options.headers[CONTENT_TYPE]??='multipart/form-data';
        const formData = new FormData();
        Object.keys(data).forEach((key)=>{
            formData.append(key,data[key] as string);
        });
        formData.append('file',file);
        formData.append('fileName',file.name);
        return request({
            method:'post',
            url,
            data: formData,
            options,
        });
    };

}
