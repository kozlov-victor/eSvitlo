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
        let restarted = false;
        for (let i=0;i<3;i++) {
            try {
                await this.post<{success:boolean}>('/ping/restart');
                restarted = true;
                break;
            }
            catch (e) {
                await Wait(1000);
            }
        }
        if (!restarted) return false;
        await Wait(1000);
        let cnt = 0;
        while (cnt<25) {
            await Wait(3000);
            try {
                const alive = (await this.health()).alive;
                if (alive) {
                    return true;
                }
                cnt++;
            }
            catch (e) {}
        }
        return false;
    }

}