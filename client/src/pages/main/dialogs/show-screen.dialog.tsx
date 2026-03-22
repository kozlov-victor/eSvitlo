import { VEngineTsxFactory } from "@engine/renderable/tsx/_genetic/vEngineTsxFactory.h";
import {Dialog} from "../../../components/modal/dialog/dialog";
import {DI} from "@engine/core/ioc";
import {MainService} from "../main.service";
import {Reactive} from "@engine/renderable/tsx/decorator/reactive";

function parseColor(hex:string) {
    const c = Number.parseInt(hex.replace('#',''), 16);
    return {
        r: (c >> 16) & 0xFF,
        g: (c >> 8)  & 0xFF,
        b:  c        & 0xFF
    };
}

@DI.Injectable()
export class ShowScreenDialog extends Dialog<{}, void>{

    private screen = {
        fullWidth: 128,
        fullHeight: 64,
        width: 62,
        height: 40,
        offsetX: 36,
        offsetY: 25,
        scale: 5,
        colorOn: parseColor('#e8f0fb'),
        colorOff: parseColor('#818181'),
    }

    private canvas: HTMLCanvasElement;
    @DI.Inject(MainService) private readonly mainService: MainService;

    @Reactive.BoundedContext()
    private async draw() {
        const buffer = await this.mainService.getScreen();
        const data = new Uint8Array(buffer);
        const ctx = this.canvas.getContext('2d')!;
        const img = ctx.createImageData(this.screen.fullWidth, 64);

        for (let x = 0; x < this.screen.fullWidth; x++) {
            for (let page = 0; page < 8; page++) {
                const byte = data[page * this.screen.fullWidth + x];

                for (let bit = 0; bit < 8; bit++) {
                    const y = page * 8 + bit;
                    const pixelOn = (byte >> bit) & 1;

                    const i = (y * this.screen.fullWidth + x) * 4;
                    const c = pixelOn ? this.screen.colorOn: this.screen.colorOff;

                    img.data[i    ] = c.r;
                    img.data[i + 1] = c.g;
                    img.data[i + 2] = c.b;
                    img.data[i + 3] = 255;
                }
            }
        }
        ctx.putImageData(img, 0, 0);
    }


    override async onMounted() {
        super.onMounted();
        await this.draw();
    }

    protected override renderDialog(): JSX.Element {
        return (
            <>
                <div style={{
                    position: 'relative',
                    width: `${this.screen.width * this.screen.scale}px`,
                    height: `${this.screen.height * this.screen.scale}px`,
                    overflow: 'hidden',
                    margin: '0 auto',
                }}>
                    <canvas
                        style={{
                            display: 'block',
                            position: 'relative',
                            left: `${-this.screen.offsetX * this.screen.scale}px`,
                            top: `${-this.screen.offsetY * this.screen.scale}px`,
                            width: `${this.screen.fullWidth * this.screen.scale}px`,
                            height: `${this.screen.fullHeight * this.screen.scale}px`,
                            imageRendering:'pixelated'
                        }}
                        width={this.screen.fullWidth} height={this.screen.fullHeight}
                        ref={el=>this.canvas = el}/>
                </div>
                <button onclick={this.draw}>Оновити</button>
            </>
        );
    }
}