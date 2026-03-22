import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {Frame} from "../../components/frame";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {StatusBar} from "../../components/statusBar";
import {DI} from "@engine/core/ioc";
import {SsidValidator} from "./ssid.validator";
import {ISsid, ITickInfo} from "./model";
import {SignalLevel} from "../../components/signal_level/SignalLevel";
import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";
import {MainService} from "./main.service";
import {Router} from "../../router";
import {DialogService} from "../../components/modal/dialog.service";
import {AuthService} from "../../service/auth.service";
import {ShowScreenDialog} from "./dialogs/show-screen.dialog";
import {InputSetterService} from "../../utils/input.setter.service";

const icon =
    <svg width={24} height={24} fill="#000000" version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"
         viewBox="0 0 512 512" xml:space="preserve">
        <path d="M512,405.854V31.219H0v374.634h187.317v37.463H143.61v37.463H368.39v-37.463h-43.707v-37.463H512z M287.219,443.317
			h-62.439v-37.463h62.439V443.317z M37.463,368.39V68.683h437.073V368.39H37.463z"/>
        <rect x="162.341" y="206.049" width="37.463" height="87.415"/>
        <rect x="237.268" y="143.61" width="37.463" height="149.854"/>
        <rect x="312.195" y="193.561" width="37.463" height="99.902"/>
</svg>

@DI.Injectable()
export class MainWidget extends BaseTsxComponent {

    @DI.Inject(SsidValidator) private readonly validator: SsidValidator;
    @DI.Inject(MainService) private readonly mainService: MainService;
    @DI.Inject(AuthService) private readonly authService: AuthService;
    @DI.Inject(Router) private readonly router: Router;
    @DI.Inject(DialogService) private readonly dialogService: DialogService;
    @DI.Inject(InputSetterService) private readonly setter: InputSetterService;

    private result = '';
    private success: boolean;
    private pending: boolean;
    private model = {} as ISsid;
    private tickInfo: ITickInfo;
    @Reactive.Property() private loading: boolean;
    private tid: any;
    @Reactive.Property() public extended = false;

    @Reactive.Method()
    override async onMounted() {
        super.onMounted();
        this.loading = true;
        try {
            this.model = await this.mainService.getSsid();
            this.loading = false;
        }
        catch (e) {
            console.log('error',e);
            this.showStatusBar('Помилка завантаження даних',false);
        }
        await this.getTickInfo();
        if (!this.tickInfo.isAccessPoint) {
            this.tid = setInterval(async ()=>{
                await this.getTickInfo();
            },10_000);
        }
    }


    override onDestroyed() {
        super.onDestroyed();
        clearInterval(this.tid);
    }

    @Reactive.Method()
    private async getTickInfo() {
        this.tickInfo = await this.mainService.getTickInfo();
    }

    private validate(extended:boolean) {
        const result = this.validator.validate(this.model,extended);
        if (!result.success) {
            this.showStatusBar(result.message,false);
        }
        else {
            this.showStatusBar('',true);
        }
        return result.success;
    }

    private leadZero(val:number) {
        return `${val<9?'0':''}${val}`;
    }

    private formatDuration(ms: number) {
        if (!ms) return '';
        if (ms < 0) ms = 0;

        const sec = Math.floor(ms / 1000);
        const days = Math.floor(sec / 86400);
        const hours = Math.floor((sec % 86400) / 3600);
        const minutes = Math.floor((sec % 3600) / 60);
        const seconds = sec % 60;

        const parts = [];
        if (days) parts.push(`${this.leadZero(days)} д`);
        if (hours) parts.push(`${this.leadZero(hours)} год`);
        if (minutes) parts.push(`${this.leadZero(minutes)} хв`);
        if (seconds || parts.length === 0) parts.push(`${this.leadZero(seconds)} сек`);

        return parts.join(" ");
    }

    @Reactive.Method()
    private setValue<K extends keyof ISsid>(e: Event, key: K, parser: (raw: string) => ISsid[K]) {
        const input = e.target as HTMLInputElement;
        this.model[key] = parser(input.value);
    }

    @Reactive.Method()
    private async save() {
        const model = this.model;
        if (!this.validate(this.extended)) return;
        try {
            this.pending = true;
            const resp = await this.mainService.save(model);
            this.showStatusBar(resp.success?'Збережено':'Помилка збереження',resp?.success ?? false);
        }
        catch (e) {
            this.showStatusBar('Помилка збереження',false);
        }
        finally {
            this.pending = false;
        }
    }

    @Reactive.Method()
    private async restart() {
        this.showStatusBar('Рестарт...',true);
        const result = await this.mainService.restart();
        await this.dialogService.alert(
            result? 'Рестарт здійснено успішно': 'Помилка рестарта'
        );
        this.showStatusBar('',true);
    }

    @Reactive.Method()
    private checkUpdate() {
        this.router.navigate('update');
    }

    @Reactive.Method()
    private logout() {
        this.authService.logout();
        this.router.navigate('login');
    }

    @Reactive.Method()
    private showStatusBar(result: string, success: boolean) {
        this.result = result;
        this.success = success;
    }

    @Reactive.BoundedContext()
    private toPersonalAccount() {
        this.router.navigate('personal-account');
    }

    @Reactive.Method()
    private async openShowScreenDialog() {
        await this.dialogService.openDialog(ShowScreenDialog);
    }

    render(): JSX.Element {

        return (
            <Frame title={'eSvitlo♥'}>
                {this.loading && <>Завантаження...</>}
                {!this.loading &&
                    <>
                        Панель керування приладом eSvitlo <span style={{cursor:'pointer'}} onclick={this.openShowScreenDialog}>{icon}</span>
                        <div style={{width:'90%',margin:'0 auto',}}>
                            <div>
                                {
                                    this.tickInfo?.signal &&
                                    <SignalLevel level={this.tickInfo.signal}/>
                                }
                            </div>
                            <table>
                                <thead>
                                <tr>
                                    <th/>
                                    <th/>
                                </tr>
                                </thead>
                                <tbody>
                                <tr>
                                    <td>
                                        WIFI мережа
                                    </td>
                                    <td>
                                        <input {...this.setter.bind(this.model,'ssid',String)}/>
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Пароль
                                    </td>
                                    <td>
                                        <input
                                            type='password'
                                            {...this.setter.bind(this.model,'password',String)}/>
                                </td>
                                </tr>
                                <tr>
                                    <td>
                                        ID приладу
                                    </td>
                                    <td>
                                        <input {...this.setter.bind(this.model,'id',String)}/>
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Лінія
                                    </td>
                                    <td>
                                        <input {...this.setter.bind(this.model,'spot',String)}/>
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        URL сервера
                                    </td>
                                    <td>
                                        <input {...this.setter.bind(this.model,'ep',String)}/>
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Періодичність пінгу (сек)
                                    </td>
                                    <td>
                                        <input
                                            type={'tel'}
                                            {...this.setter.bind(this.model,'time',Number)}/>
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Розширені налаштування
                                    </td>
                                    <td>
                                        <input
                                            type={'checkbox'}
                                            {...this.setter.bind(this,'extended',Boolean)}/>
                                    </td>
                                </tr>
                                {this.extended &&
                                    <tr>
                                        <td>
                                            Control PIN
                                        </td>
                                        <td>
                                            <input
                                                type={'tel'}
                                                {...this.setter.bind(this.model,'controlPin',Number)}/>
                                        </td>
                                    </tr>
                                }
                                </tbody>
                            </table>
                        </div>
                        <button onclick={this.save}>Зберегти</button>
                    </>
                }
                {this.tickInfo &&
                    <div className={'tickInfo'}>
                        <div>Режим роботи: {this.tickInfo.isAccessPoint?'Точка доступу':'WIFI'}</div>
                        {!this.tickInfo.isAccessPoint &&
                            <>
                                <div>Остання відповідь сервера: {this.tickInfo.lastPingResponse}</div>
                                <div>Тік: {this.tickInfo.tick}</div>
                                <div>Помилок: {this.tickInfo.errorCnt}</div>
                                <div>Система працює: {this.formatDuration(this.tickInfo.time)}</div>
                            </>
                        }
                    </div>
                }
                {this.pending && <>Запит в обробці...</>}
                {this.result && <StatusBar success={this.success} text={this.result}/>}

                <hr/>
                <button onclick={this.restart}>Рестарт</button>
                {
                    this.tickInfo?.isAccessPoint===false &&
                    <button onclick={this.checkUpdate}>Перевірити оновлення</button>
                }
                {
                    this.tickInfo?.isAccessPoint===true &&
                    <button onclick={this.toPersonalAccount}>Змінити пароль адміністратора</button>
                }
                <button onclick={this.logout}>Вийти</button>

            </Frame>
        );
    }


}
