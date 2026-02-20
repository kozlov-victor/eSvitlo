import {DI} from "@engine/core/ioc";
import {ILastVersion, IVersion} from "../main/model";
import {BaseService} from "../../service/base.service";

export interface IOtaProgressResult {
    completed: boolean;
    progress: boolean;
    success: boolean;
    body: string;
}

@DI.Injectable()
export class UpdateService extends BaseService{

    public async otaVersion() {
        //await Wait(1000); return {version: '1.2.3'} as IVersion;
        return await this.post<IVersion>('/ota/version');
    }

    public async otaUpdate() {
        //await Wait(1000); return {version: '1.3.4',success: true,error:''} as ILastVersion;
        return await this.post<ILastVersion>('/ota/update');
    }

    public async otaUpgrade(onProgress:(data:IOtaProgressResult)=>void) {

        // for (let i=0;i<10;i++) {
        //     await Wait(2000);
        //     onProgress({progress:true,body:`${i*10}`,success:true,completed:false});
        // }

        return await this.post<IOtaProgressResult>(
            '/ota/upgrade',undefined,
            {
                headers: {'Content-Type':'text/event-stream'},
                onProgress: (data) => {
                    onProgress(data?.data);
                }
            }
        );
    }
}