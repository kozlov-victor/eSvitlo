
export interface ILogin {
    login: string;
    password: string;
}

export interface ISsid {
    ssid:string;
    password:string;
    id:string;
    ep:string;
    time:number;
    spot: string;
    controlPin: number;
}

export interface ITickInfo {
    lastPingResponse: string;
    tick: number;
    errorCnt: number;
    time: number;
    isAccessPoint: boolean;
    signal: number;
}

export interface IVersion {
    version: string;
}

export interface ILastVersion {
    success: boolean;
    error: string;
    version: string;
}
