export interface ISsid {
    ssid:string;
    password:string;
    id:string;
    ep:string;
    time:number;
    spot: string;
}

export interface ITickInfo {
    lastPingResponse: string;
    tick: number;
    time: number;
    isAccessPoint: boolean;
    signal: number;
}
