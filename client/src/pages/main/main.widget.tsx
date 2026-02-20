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

@DI.Injectable()
export class MainWidget extends BaseTsxComponent {

    @DI.Inject(SsidValidator) private readonly validator: SsidValidator;
    @DI.Inject(MainService) private readonly mainService: MainService;
    @DI.Inject(AuthService) private readonly authService: AuthService;
    @DI.Inject(Router) private readonly router: Router;
    @DI.Inject(DialogService) private readonly dialogService: DialogService;

    private result = '';
    private success: boolean;
    private pending: boolean;
    private model = {} as ISsid;
    private tickInfo: ITickInfo;
    @Reactive.Property() private loading: boolean;
    private tid: any;

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

    private validate() {
        const result = this.validator.validate(this.model);
        if (!result.success) {
            this.showStatusBar(result.message,false);
        }
        else {
            this.showStatusBar('',true);
        }
        return result.success;
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
        if (days) parts.push(`${days} д`);
        if (hours) parts.push(`${hours} год`);
        if (minutes) parts.push(`${minutes} хв`);
        if (seconds || parts.length === 0) parts.push(`${seconds} сек`);

        return parts.join(" ");
    }

    @Reactive.Method()
    private setValue<K extends keyof ISsid>(e: Event, key: K, parser: (raw: string) => ISsid[K]) {
        const input = e.target as HTMLInputElement;
        this.model[key] = parser(input.value);
    }

    @Reactive.Method()
    private async save(e:Event) {
        const model = this.model;
        if (!this.validate()) return;
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

    render(): JSX.Element {

        return (
            <Frame title={'eSvitlo♥'}>
                {this.loading && <>Завантаження...</>}
                {!this.loading &&
                    <>
                        Панель керування приладом eSvitlo
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
                                        <input
                                            value={this.model.ssid}
                                            onchange={e => this.setValue(e,'ssid',v => v)}
                                        />
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Пароль
                                    </td>
                                    <td>
                                        <input
                                            type='password'
                                            value={this.model.password}
                                            onchange={e => this.setValue(e,'password',v => v)}
                                        />
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        ID приладу
                                    </td>
                                    <td>
                                        <input
                                            value={this.model.id}
                                            onchange={e => this.setValue(e,'id',v => v)}
                                        />
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Лінія
                                    </td>
                                    <td>
                                        <input
                                            value={this.model.spot}
                                            onchange={e => this.setValue(e,'spot',v => v)}
                                        />
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        URL сервера
                                    </td>
                                    <td>
                                        <input
                                            value={this.model.ep}
                                            onchange={e => this.setValue(e,'ep',v => v)}
                                        />
                                    </td>
                                </tr>
                                <tr>
                                    <td>
                                        Періодичність пінгу (сек)
                                    </td>
                                    <td>
                                        <input
                                            value={''+this.model.time}
                                            type={'tel'}
                                            onchange={e => this.setValue(e,'time',Number)}
                                        />
                                    </td>
                                </tr>
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
