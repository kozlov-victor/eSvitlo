import {DI} from "@engine/core/ioc";
import {HttpClient} from "../../httpClient";
import {ISsid, ITickInfo} from "./model";
import {DialogService} from "../../components/modal/dialog.service";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {BaseService} from "../../service/base.service";

export const Wait = async (time:number)=>{
    return new Promise<void>(resolve=>{
        setTimeout(resolve,time);
    });
}


@DI.Injectable()
export class MainService extends BaseService {

    public async getSsid() {
        return await this.get<ISsid>('/ssid/get');
    }

    public async getTickInfo() {
        return await this.get<ITickInfo>('/ping/getTickInfo');
    }

    public async save(model:ISsid) {
        return await this.post<{success:boolean}>('/ssid/save',model);
    }


    public async health() {
        //await Wait(1000); return {alive:true};
        return await this.post<{alive:boolean}>('/ping/health');
    }

    public async restart() {
        try {
            await this.post<{success:boolean}>('/ping/restart');
        }
        catch (e) {
            console.error(e);
        }
        finally {
            let cnt = 0;
            await Wait(1000);
            while (cnt<25) {
                await Wait(3000);
                const alive = (await this.health()).alive;
                if (alive) {
                    break;
                }
                cnt++;
            }
        }
    }

}