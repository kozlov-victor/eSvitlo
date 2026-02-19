import {VEngineTsxFactory} from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";
import {Dialog} from "../../../components/modal/dialog/dialog";

export class ResetAdminDialog extends Dialog<{}, string> {

    private login: string;

    @Reactive.Method()
    private setLogin(e:Event & {target: HTMLInputElement}) {
        this.login = e.target.value;
    }

    protected override renderDialog(): JSX.Element {
        return (
            <>
                <div>
                    <div>Заводський пароль</div>
                    <input
                        style={{width: `150px`}}
                        type={'password'} value={this.login} oninput={this.setLogin}/>
                </div>
                <div>
                    <button
                        disabled={!this.login}
                        onclick={e=>this.close(this.login)}
                    >Відновити заводські налаштування доступу</button>
                </div>
            </>
        );
    }
}