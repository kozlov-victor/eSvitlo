import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {Frame} from "../components/frame";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {DomRootComponent} from "@engine/renderable/tsx/dom/domRootComponent";
import {HttpClient} from "../httpClient";
import {StatusBar} from "../components/statusBar";
import {DI} from "@engine/core/ioc";
import {SsidValidator} from "./ssid.validator";
import {ISsid, ITickInfo} from "./model";
import {SignalLevel} from "../components/signal_level/SignalLevel";

@DI.CSS('./main.widget.css')
export class MainWidget extends DomRootComponent {

    @DI.Inject(SsidValidator)
    private readonly validator: SsidValidator;

    private result = '';
    private success: boolean;
    private pending: boolean;
    private model = {} as ISsid;
    private tickInfo: ITickInfo;
    @Reactive.Property() private loading: boolean;

    @Reactive.Method()
    override async onMounted() {
        super.onMounted();
        this.loading = true;
        try {
            this.model = await HttpClient.get<ISsid>('/ssid/get');
            this.loading = false;
        }
        catch (e) {
            console.log('error',e);
            this.success = false;
            this.result = 'Помилка завантаження даних';
        }
        await this.getTickInfo();
        if (!this.tickInfo.isAccessPoint) {
            setInterval(async ()=>{
                await this.getTickInfo();
            },10_000);
        }
    }

    @Reactive.Method()
    private async getTickInfo() {
        this.tickInfo = (await HttpClient.get<ITickInfo>('/ping/getTickInfo'));
    }

    private validate() {
        const result = this.validator.validate(this.model);
        if (!result.success) {
            this.success = false;
            this.result = result.message;
        }
        else {
            this.success = true;
            this.result = '';
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
            const resp = await HttpClient.post<{success:boolean}>('/ssid/save',model);
            this.success = resp?.success ?? false;
            this.result = this.success?'Збережено':'Помилка збереження';
        }
        catch (e) {
            this.success = false;
            this.result = 'Помилка збереження';
        }
        finally {
            this.pending = false;
        }
    }

    private async restart() {
        await HttpClient.post<{success:boolean}>('/ping/restart');
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
                                            onchange={e => this.setValue(e,'time',Number)}
                                        />
                                    </td>
                                </tr>
                                </tbody>
                            </table>
                        </div>
                        <button onclick={this.save}>Зберегти</button>
                        <button onclick={this.restart}>Рестарт</button>
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
            </Frame>
        );
    }


}
