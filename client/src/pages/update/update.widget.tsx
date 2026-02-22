import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";
import {DI} from "@engine/core/ioc";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {UpdateService} from "./update.service";
import {DialogService} from "../../components/modal/dialog.service";
import {MainService, Wait} from "../main/main.service";
import {ILastVersion} from "../main/model";
import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {Frame} from "../../components/frame";
import {Router} from "../../router";

@DI.Injectable()
export class UpdateWidget extends BaseTsxComponent {

    @DI.Inject(UpdateService) private readonly updateService: UpdateService;
    @DI.Inject(MainService) private readonly mainService: MainService;
    @DI.Inject(DialogService) private readonly dialogService: DialogService;
    @DI.Inject(Router) private readonly router: Router;

    private loading = true;
    private version: string;
    private lastVersion: ILastVersion;


    override async onMounted() {
        super.onMounted();
        await this.checkVersion();
    }

    @Reactive.Method()
    private async checkVersion() {
        try {
            this.version = (await this.updateService.otaVersion()).version;
            this.lastVersion = await this.updateService.otaUpdate();
        }
        catch (e) {
            console.error(e);
        }
        finally {
            if (!this.lastVersion) this.lastVersion = {success:false,version:'?',error:'version error'};
            this.loading = false;
        }
        if (!this.lastVersion.success) {
            await this.dialogService.alert(this.lastVersion.error);
        }
    }

    private getProgress(percent: number) {
        const width = 300;
        return (
            <div>
                <div>{percent}%</div>
                <div style={{margin: '0 auto',width:`${width}px`,border:`1px solid gray`,height:`20px`}}>
                    <div
                        style={{width:`${width * percent / 100}px`,backgroundColor:'gray',height:`20px`}}
                    />
                </div>
            </div>
        );
    }

    @Reactive.BoundedContext()
    private async otaUpgrade() {
        const prompt = await this.dialogService.prompt(`Буде встановлене оновлення. не вимикайте прилад`,['Ok','Пізніше']);
        if (!prompt) return;
        let ref = this.dialogService.alertSync('Виконується оновлення...',false);
        try {
            const result = await this.updateService.otaUpgrade(data=>{
                ref.close();
                if (data.progress) {
                    ref = this.dialogService.alertSync(this.getProgress(+data.body),false);
                }
                else {
                    ref = this.dialogService.alertSync(data.body, false);
                }
            });
            console.log(result);
            ref.close();
            if (result.completed && result.success) {
                ref = this.dialogService.alertSync('Оновлення встановлено. Рестарт системи...',false);
                await Wait(1000);
                const restarted = await this.mainService.restart();
                ref.close();
                if (restarted) {
                    ref = this.dialogService.alertSync('Оновлення виконано успішно!',false);
                    location.reload();
                }
                else {
                    await this.dialogService.alert(`Помилка автоматичного рестарта. Здійсність рестарт самостійно`);
                }
            }
            else {
                await this.dialogService.alert(result.body);
            }
        }
        catch (e:any) {
            // alert(e?.message ?? e.error ?? e);
            ref.close();
            await this.dialogService.alert('Помилка оновлення. Спробуйте ще раз');
        }
    }

    private canUpdate() {
        return true;//return this.version && this.lastVersion.success && this.lastVersion.version!==this.version;
    }

    @Reactive.Method()
    private back() {
        this.router.navigate('/');
    }


    render(): JSX.Element {
        return (
            <Frame title={'eSvitlo♥'}>
                {this.loading && <>Завантаження...</>}
                {!this.loading &&
                <>
                    <div>Версія прошивки: {this.version}</div>
                    <div>Остання доступна версія: {this.lastVersion.version}</div>
                    {
                        this.canUpdate() &&
                        <button onclick={this.otaUpgrade}>Встановити оновлення</button>
                    }
                    <button onclick={this.back}>На головну</button>
                </>
                }
            </Frame>
        );
    }



}