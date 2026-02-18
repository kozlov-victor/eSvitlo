import { DI } from "@engine/core/ioc";
import { VEngineTsxFactory } from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {BaseTsxComponent} from "@engine/renderable/tsx/base/baseTsxComponent";
import {Frame} from "../../components/frame";
import {Router} from "../../router";
import {ILogin} from "../main/model";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {AuthService} from "../../service/auth.service";
import {DialogService} from "../../components/modal/dialog.service";

@DI.Injectable()
export class PersonalAccountWidget extends BaseTsxComponent {

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
    private valid() {
        return this.model.login && this.model.password;
    }

    @Reactive.Method()
    private async setCreds(e:Event) {
        e.preventDefault();
        e.stopPropagation();
        try {
            await this.authService.setCredentials(this.model.login,this.model.password);
            await this.authService.createToken(this.model.login, this.model.password);
            this.router.navigate('/');
        }
        catch (e) {
            await this.dialogService.alert('Помилка зміни даних адміністратора');
        }
    }

    render(): JSX.Element {
        return (
            <Frame title={'eSvitlo♥'}>
                Встановідь нові дані облікового запису адміністратора приладу
                <form autocomplete={'on'}>
                    <table>
                        <tr>
                            <td>Аміністратор</td>
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
                        onclick={this.setCreds}
                        disabled={!this.valid}>Ok</button>
                </form>
            </Frame>
        );
    }

}