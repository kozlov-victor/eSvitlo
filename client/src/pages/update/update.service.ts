import {DI} from "@engine/core/ioc";
import {ILastVersion, IVersion} from "../main/model";
import {BaseService} from "../../service/base.service";

@DI.Injectable()
export class UpdateService extends BaseService{

    public async otaVersion() {
        //await Wait(1000); return {version: '1.2.3'} as IVersion;
        return await this.post<IVersion>('/ota/version');
    }

    public async otaLastVersion() {
        //await Wait(1000); return {version: '1.3.4',success: true,error:''} as ILastVersion;
        return await this.post<ILastVersion>('/ota/lastVersion');
    }

    public async otaUpdate() {
        //await Wait(1000); return {success: true,status:''};
        return await this.post<{success:boolean,status:string}>('/ota/update');
    }
}