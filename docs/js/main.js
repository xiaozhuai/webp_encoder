const WEBP_ENCODE_RET_UINT8ARRAY = 0;
const WEBP_ENCODE_RET_BLOB = 1;
const WEBP_ENCODE_RET_URL = 2;

const ImageFrame = {
    template: `
        <div class="image-frame">
            <img :src="src">
            <div class="name">{{name}}</div>
            <el-form label-width="100px" label-position="left" size="mini">
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
        },
    },
}
Vue.component('ImageFrame', ImageFrame);

const App = {
    template: `
        <div id="app"
            @drop.prevent.self.stop="onDropFiles"
            @dragover.prevent.self.stop=""
            @dragenter.prevent.self.stop="dragging = true;"
            @dragleave.prevent.self.stop="dragging = false;">
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
                        <div style="margin-top: 16px; text-align: center;">
                            <el-button @click="clear" size="mini">Clear</el-button>
                            <el-button @click="genWebp" size="mini" :disabled="loading">Generate!</el-button>
                        </div>
                        <div v-if="webp.src !== ''" class="webp-info">Size: {{readableWebpSize}}</div>
                    </el-form>
                </div>
                <div class="control-panel">
                    <div class="title">Batch Frame Options</div>
                    <el-form label-width="100px" label-position="left" size="mini">
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
                <div class="webp-container" v-loading="loading">
                    <img v-if="webp.src !== ''" :src="webp.src" @click="downloadWebp"/>
                </div>
            </div>
            <div class="frames-container">
                <!-- TODO 为空时显示，需要先改为flex布局 -->
                <!--<el-empty description="Drop image frames here"/>-->
                <!-- TODO list懒加载 -->
                <image-frame
                    v-for="frame of frames"
                    :src="frame.src"
                    :name="frame.name"
                    v-model="frame.options"/>
            </div>
            <!-- TODO 修复拖拽时遮罩一直闪的问题 -->
            <div class="drag-layer" v-if="dragging">Drop Files Here!</div>
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
            webp: {
                src: '',
                size: 0,
            },
            loading: true,
            dragging: false,
        };
    },
    computed: {
        readableWebpSize() {
            return this.toReadableSize(this.webp.size);
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
        this.$notify.info({
            title: 'Hint',
            message: 'Drop image frames here to generate a webp!',
            duration: 0
        });
    },
    methods: {
        sortFrames(frames) {
            return frames.sort((a, b) => {
                return a.name.localeCompare(b.name);
            });
        },
        onDropFiles(evt) {
            this.dragging = false;
            let files = evt.dataTransfer.files;
            let images = [];
            for (let file of files) {
                let src = URL.createObjectURL(file);
                let name = file.name;
                images.push({src, name});
            }
            this.onImages(images);
        },
        onImages(images) {
            let frames = [];
            for (let image of images) {
                frames.push({
                    ...image,
                    options: {...this.frameOptions},
                });
            }
            this.frames = this.sortFrames(frames);
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
        /**
         *
         * @param frames            Array<{src: string, options: Object}>
         * @param fileOptions       Object
         * @param returnType        0: Uint8Array
         *                          1: {blob, size}
         *                          2: {url, size}
         * @returns {Promise<Uint8Array|{blob: Blob, size: number}|{url: string, size: number}>}
         */
        async encodeWebp(frames, fileOptions = {}, returnType = 0) {
            let m = await WebpEncoder();
            let encoder = new m.WebpEncoder();
            encoder.init(fileOptions);

            for (let frame of frames) {
                let image = await this.loadImage(frame.src);
                let imageData = this.getImageData(image);
                let rgbaPixels = new Uint8Array(imageData.data.buffer);
                encoder.push(rgbaPixels, image.naturalWidth, image.naturalHeight, frame.options);
            }
            let bytes = encoder.encode();
            switch (returnType) {
                case 0: {
                    bytes = new Uint8Array(bytes);
                    encoder.release();
                    return bytes;
                }
                case 1: {
                    let blob = new Blob([bytes], {type: 'image/webp'});
                    let size = bytes.length;
                    encoder.release();
                    return {blob, size};
                }
                case 2: {
                    let blob = new Blob([bytes], {type: 'image/webp'});
                    let size = bytes.length;
                    encoder.release();
                    let url = URL.createObjectURL(blob);
                    return {url, size};
                }
                default: {
                    throw new Error(`Invalid return type ${returnType}`)
                }
            }
        },
        batchChangeFrameOption(key) {
            let value = this.batchFrameOptions[key];
            let frames = [...this.frames];
            for (let frame of frames) {
                frame.options[key] = value;
            }
        },
        toReadableSize(bytes) {
            if (isNaN(parseFloat(bytes)) || !isFinite(bytes)) return '-';
            if (bytes === 0) return 0 + ' bytes';
            let precision = 2;
            let units = ['bytes', 'kB', 'MB', 'GB', 'TB', 'PB'];
            let number = Math.floor(Math.log(bytes) / Math.log(1024));
            return (bytes / Math.pow(1024, Math.floor(number))).toFixed(precision) + ' ' + units[number];
        },
        clear() {
            this.frames = [];
            this.webp = {
                src: '',
                size: 0,
            };
        },
        async genWebp() {
            if (this.frames.length === 0) {
                this.$message.error('No frames, please drop image frame to continue');
                return;
            }
            this.loading = true;
            this.webp = {
                src: '',
                size: 0,
            };
            let {url, size} = await this.encodeWebp(this.frames, this.fileOptions, WEBP_ENCODE_RET_URL);
            this.webp = {src: url, size};
            this.loading = false;
        },
        downloadWebp() {
            const a = document.createElement('a');
            a.href = this.webp.src;
            a.setAttribute('download', 'output.webp');
            a.click();
        },
    },
}

new Vue({
    render: h => h(App),
}).$mount('#app');
