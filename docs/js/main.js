const ImageFrame = {
    template: `
        <div class="image-frame">
            <img :src="src">
            <div class="name">{{name}}</div>
            <el-form label-width="120px" label-position="left" size="mini">
                <el-form-item label="Duration (ms)">
                    <el-input-number
                        :value="value.duration" 
                        :step="1" 
                        @change="change('duration', $event)"/>
                </el-form-item>
                <el-form-item label="Quality">
                    <el-input-number 
                        :value="value.quality"
                        :min="0" :max="100" :step="0.1"
                        @change="change('quality', $event)"/>
                </el-form-item>
                <el-form-item label="Method">
                    <el-input-number
                        :value="value.method"
                        :min="0" :max="6" :step="1"
                        @change="change('method', $event)"/>
                </el-form-item>
                <el-form-item label="Lossless">
                    <el-switch
                        :value="value.lossless" 
                        @change="change('lossless', $event)"/>
                </el-form-item>
            </el-form>
        <div>
    `,
    props: {
        src: {
            type: String,
            required: true
        },
        name: {
            type: String,
            required: true
        },
        value: {
            type: Object,
            required: true,
        }
    },
    methods: {
        change(key, value) {
            let options = {...this.value};
            options[key] = value;
            this.$emit('change', options)
            this.$emit('input', options)
        }
    },
}
Vue.component('ImageFrame', ImageFrame);

const App = {
    template: `
        <div id="app">
            <div class="top-panel">
                <div class="control-panel">
                    <div class="title">File Options</div>
                    <el-form label-width="80px" label-position="left" size="mini">
                        <el-form-item label="Loop">
                            <el-input-number
                                v-model="fileOptions.loop"
                                :step="1" :min="0"/>
                        </el-form-item>
                        <el-form-item label="Kmin">
                            <el-input-number
                                v-model="fileOptions.kmin"
                                :step="1" :min="0"/>
                        </el-form-item>
                        <el-form-item label="Kmax">
                            <el-input-number
                                v-model="fileOptions.kmax"
                                :step="1" :min="0"/>
                        </el-form-item>
                        <el-form-item label="Minimize">
                            <el-switch v-model="fileOptions.minimize"/>
                        </el-form-item>
                        <el-form-item label="Mixed">
                            <el-switch v-model="fileOptions.mixed"/>
                        </el-form-item>
                        <el-form-item>
                            <el-button @click="genWebp">GO</el-button>
                        </el-form-item>
                        <div v-if="webpSrc !== ''" class="webp-info">Size: {{readableWebpSize}}</div>
                    </el-form>
                </div>
                <div class="control-panel">
                    <div class="title">Batch Frame Options</div>
                    <el-form label-width="120px" label-position="left" size="mini">
                        <el-form-item label="Duration (ms)">
                            <el-input-number 
                                v-model="batchFrameOptions.duration"
                                :step="1"/>
                            <el-button icon="el-icon-check" @click="batchChangeFrameOption('duration')"></el-button>
                        </el-form-item>
                        <el-form-item label="Quality">
                            <el-input-number
                                v-model="batchFrameOptions.quality"
                                :min="0" :max="100" :step="0.1"/>
                            <el-button icon="el-icon-check" @click="batchChangeFrameOption('quality')"></el-button>
                        </el-form-item>
                        <el-form-item label="Method">
                            <el-input-number 
                                v-model="batchFrameOptions.method"
                                :min="0" :max="6" :step="1"/>
                            <el-button icon="el-icon-check" @click="batchChangeFrameOption('method')"></el-button>
                        </el-form-item>
                        <el-form-item label="Lossless">
                            <el-switch v-model="batchFrameOptions.lossless"/>
                            <el-button icon="el-icon-check" @click="batchChangeFrameOption('lossless')"></el-button>
                        </el-form-item>
                    </el-form>
                </div>
                <div class="webp-container" v-loading="webpSrc === ''">
                    <img v-if="webpSrc !== ''" :src="webpSrc"/>
                </div>
            </div>
            <div class="frames-container">
                <image-frame v-for="frame of frames" :src="frame.src" :name="frame.name" v-model="frame.options"/>
            </div>
        </div>
    `,
    data() {
        return {
            fileOptions: {
                loop: 0,
                kmax: 0,
                kmin: 0,
                minimize: true,
                mixed: true,
            },
            frameOptions: {
                duration: 100,
                quality: 100,
                method: 0,
                lossless: false,
            },
            batchFrameOptions: {
                duration: 100,
                quality: 100,
                method: 0,
                lossless: false,
            },
            frames: [],
            webpSrc: '',
            webpSize: 0,
        };
    },
    computed: {
        readableWebpSize() {
            let bytes = this.webpSize;
            if (isNaN(parseFloat(bytes)) || !isFinite(bytes)) return '-';
            let precision = 2;
            let units = ['bytes', 'kB', 'MB', 'GB', 'TB', 'PB'];
            let number = Math.floor(Math.log(bytes) / Math.log(1024));
            return (bytes / Math.pow(1024, Math.floor(number))).toFixed(precision) +  ' ' + units[number];
        }
    },
    async mounted() {
        this.onImages([
            {
                src: 'frames/frame_0.jpg',
                name: 'frame_0.jpg',
            },
            {
                src: 'frames/frame_1.jpg',
                name: 'frame_1.jpg',
            },
            {
                src: 'frames/frame_2.jpg',
                name: 'frame_2.jpg',
            },
            {
                src: 'frames/frame_3.jpg',
                name: 'frame_3.jpg',
            },
        ]);
        await this.genWebp();
    },
    methods: {
        sortFrames(frames) {
            // TODO
            return frames;
        },
        onImages(images) {
            let frames = [];
            for (let image of images) {
                frames.push({
                    ...image,
                    options: {...this.frameOptions},
                });
            }
            frames = this.sortFrames(frames);
            this.frames = frames;
        },
        async loadImage(url) {
            const image = new Image();
            await new Promise((resolve, reject) => {
                image.onload = resolve;
                image.onerror = () => {
                    reject(new Error(`Error load image ${url}`));
                };
                image.src = url;
            });
            return image;
        },
        getImageData(image) {
            if (!window.getImageDataCanvas) {
                window.getImageDataCanvas = document.createElement('canvas');
                window.getImageDataCanvasCtx = window.getImageDataCanvas.getContext('2d', {
                    willReadFrequently: true,
                });
            }
            let canvas = window.getImageDataCanvas;
            let context = window.getImageDataCanvasCtx;
            canvas.width = image.naturalWidth;
            canvas.height = image.naturalHeight;
            context.drawImage(image, 0, 0);
            return context.getImageData(0, 0, image.naturalWidth, image.naturalHeight);
        },
        async encodeWebp(frames, fileOptions = {}) {
            let m = await WebpEncoder();
            let encoder = new m.WebpEncoder();
            encoder.Init(fileOptions);

            for (let frame of frames) {
                let image = await this.loadImage(frame.src);
                let imageData = this.getImageData(image);
                let rgbaPixels = new Uint8Array(imageData.data.buffer);
                encoder.AddFrame(rgbaPixels, image.naturalWidth, image.naturalHeight, frame.options);
            }
            let bytes = encoder.Encode();
            let size = bytes.length;
            let blob = new Blob([bytes], {type: 'image/webp'});
            encoder.Release();
            return {size, blob};
        },
        async genWebp() {
            this.webpSrc = '';
            this.webpSize = 0;
            let {size, blob} = await this.encodeWebp(this.frames, this.fileOptions);
            this.webpSrc = URL.createObjectURL(blob);
            this.webpSize = size;
        },
        batchChangeFrameOption(key) {
            let value = this.batchFrameOptions[key];
            let frames = [...this.frames];
            for (let frame of frames) {
                frame.options[key] = value;
            }
            console.log(`batchChangeFrameOption ${key}: ${value}`);
        },
    },
}

new Vue({
    render: h => h(App),
}).$mount('#app');
