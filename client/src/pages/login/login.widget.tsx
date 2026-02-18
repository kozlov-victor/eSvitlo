import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";
import {DI} from "@engine/core/ioc";
import {Frame} from "../../components/frame";
import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {ILogin, ISsid} from "../main/model";
import {AuthService} from "../../service/auth.service";
import {DialogService} from "../../components/modal/dialog.service";
import {Router} from "../../router";
import {HttpClient} from "../../httpClient";
import {ResetAdminDialog} from "./dialog/reset-admin.dialog";

@DI.Injectable()
export class LoginWidget extends BaseTsxComponent {

    @DI.Inject(AuthService) private readonly authService: AuthService;
    @DI.Inject(DialogService) private readonly dialogService: DialogService;
    @DI.Inject(Router) private readonly router: Router;
    private model: ILogin = {login: '',password: ''};

    @Reactive.Method()
    private setValue<K extends keyof ILogin>(e: Event, key: K, parser: (raw: string) => ILogin[K]) {
        const input = e.target as HTMLInputElement;
        this.model[key] = parser(input.value);
    }

    @Reactive.BoundedContext()
    private canLogin() {
        return this.model.login && this.model.password;
    }

    @Reactive.Method()
    private async login(e:Event) {
        e.preventDefault();
        e.stopPropagation();
        try {
            await this.authService.createToken(this.model.login, this.model.password);
            this.router.navigate('/');
        }
        catch (e) {
            await this.dialogService.alert('Помилка авторизації');
        }
    }

    @Reactive.Method()
    private async reset(e:Event)  {
        e.preventDefault();
        e.stopPropagation();
        const password = await this.dialogService.openDialog(ResetAdminDialog);
        if (!password) return;
        try {
            await HttpClient.post<void>('/personal-account/reset',{password});
            await this.dialogService.alert('Встановлено заводський пароль за замовченням');
            this.router.navigate('login');
        }
        catch (e: any) {
            if (e?.status === 403) {
                await this.dialogService.alert('Заборонено в режимі WIFI');
            }
            else if (e?.status === 401) {
                await this.dialogService.alert('Невірний пароль');
            }
        }
    }

    override render(): JSX.Element {
        return (
            <Frame title={'eSvitlo♥'}>
                <form autocomplete={'on'}>
                    <table>
                        <tr>
                            <td>Логін</td>
                            <td>
                                <input
                                    value={''+this.model.login}
                                    name={'login'}
                                    autocomplete={'username'}
                                    onchange={e => this.setValue(e,'login',v=>v)}
                                />
                            </td>
                        </tr>
                        <tr>
                            <td>Пароль</td>
                            <td>
                                <input
                                    value={''+this.model.password}
                                    name={'password'}
                                    type={'password'}
                                    autocomplete='current-password'
                                    onchange={e => this.setValue(e,'password',v=>v)}
                                />
                            </td>
                        </tr>
                    </table>
                    <button
                        onclick={this.login}
                        disabled={!this.canLogin}>Ok</button>

                    <hr/>

                    <button onclick={this.reset}>Скинути пароль</button>

                </form>
            </Frame>
        );
    }

}