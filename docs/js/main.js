function modifyImageUrl(el, url) {
    if (el.tagName.toLowerCase() === "img") {
        el.setAttribute("src", url);
    } else {
        el.style.backgroundImage = `url(${url})`;
    }
}

// 自定义图片懒加载指令
Vue.directive('lazy-image', {
    bind(el, {value = ""}) {
        if (!window.IntersectionObserver) {
            modifyImageUrl(el, value);
        } else {
            let observer = new IntersectionObserver(function (entries) {
                entries.forEach(entry => {
                    if (typeof entry.isIntersecting === "undefined") {
                        modifyImageUrl(el, value);
                    } else if (entry.isIntersecting) {
                        observer.unobserve(entry.target);
                        modifyImageUrl(entry.target, value)
                    }
                })
            });
            observer.observe(el);
        }
    },
});

const WebpEncoder = {
    ENCODE_RET_UINT8ARRAY: 0,
    ENCODE_RET_BLOB: 1,
    ENCODE_RET_URL: 2,
    _module: null,
    _getImageDataCanvas: null,
    _getImageDataCanvasCtx: null,
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
        if (!this._getImageDataCanvas) {
            this._getImageDataCanvas = document.createElement('canvas');
            this._getImageDataCanvasCtx = this._getImageDataCanvas.getContext('2d', {
                willReadFrequently: true,
            });
        }
        let canvas = this._getImageDataCanvas;
        let context = this._getImageDataCanvasCtx;
        canvas.width = image.naturalWidth;
        canvas.height = image.naturalHeight;
        context.drawImage(image, 0, 0);
        return context.getImageData(0, 0, image.naturalWidth, image.naturalHeight);
    },
    /**
     *
     * @param frames            Array<{src: string, options: Object}>
     * @param fileOptions       Object
     * @param callback          function(progress: number)
     * @param returnType        0: Uint8Array
     *                          1: {blob, size}
     *                          2: {url, size}
     * @returns {Promise<Uint8Array|{blob: Blob, size: number}|{url: string, size: number}>}
     */
    async encode(frames, callback, fileOptions = {}, returnType = 0) {
        await callback(0);
        if (!this._module) {
            this._module = await WebpEncoderWasm();
        }
        let encoder = new this._module.WebpEncoder();
        encoder.init(fileOptions);
        await callback(5);

        let c = 0;
        for (let frame of frames) {
            let image = await this.loadImage(frame.src);
            let imageData = this.getImageData(image);
            let rgbaPixels = new Uint8Array(imageData.data.buffer);
            await encoder.push(rgbaPixels, image.naturalWidth, image.naturalHeight, frame.options);
            c++;
            await callback(c / frames.length * 90 + 5);
        }

        await callback(95);
        let bytes = encoder.encode();
        switch (returnType) {
            case 0: {
                bytes = new Uint8Array(bytes);
                encoder.release();
                await callback(100);
                return bytes;
            }
            case 1: {
                let blob = new Blob([bytes], {type: 'image/webp'});
                let size = bytes.length;
                encoder.release();
                await callback(100);
                return {blob, size};
            }
            case 2: {
                let blob = new Blob([bytes], {type: 'image/webp'});
                let size = bytes.length;
                encoder.release();
                let url = URL.createObjectURL(blob);
                await callback(100);
                return {url, size};
            }
            default: {
                throw new Error(`Invalid return type ${returnType}`)
            }
        }
    },
}

const ImageFrame = {
    template: `
        <div class="image-frame">
            <img v-lazy-image="src">
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
                <el-form-item label="Exact">
                    <el-switch
                        :value="value.exact" 
                        @change="change('exact', $event)"/>
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
            this.$emit('change', options);
            this.$emit('input', options);
        },
    },
};
Vue.component('ImageFrame', ImageFrame);

const App = {
    template: `
        <div id="app"
            @dragenter.prevent.stop="dragging = true;">
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
                            <el-button @click="genWebp" size="mini" :disabled="loading">Encode!</el-button>
                        </div>
                        <div v-if="webp.src !== ''" class="webp-info">Size: {{readableWebpSize}}</div>
                        <div v-if="loading" class="webp-info">
                            <el-progress :percentage="progress" :format="progress + ''"/>
                        </div>
                    </el-form>
                </div>
                <div class="control-panel">
                    <div class="title">Batch Frame Options</div>
                    <el-form label-width="100px" label-position="left" size="mini">
                        <el-form-item label="Duration (ms)">
                            <el-input-number 
                                v-model="batchFrameOptions.duration"
                                :step="1"
                                @change="batchFrameOptionsChanged.duration = true"/>
                            <el-button
                                icon="el-icon-check" plain
                                :type="batchFrameOptionsChanged.duration ? 'danger' : ''"
                                @click="batchChangeFrameOption('duration')"/>
                        </el-form-item>
                        <el-form-item label="Quality">
                            <el-input-number
                                v-model="batchFrameOptions.quality"
                                :min="0" :max="100" :step="0.1"
                                @change="batchFrameOptionsChanged.quality = true"/>
                            <el-button
                                icon="el-icon-check" plain
                                :type="batchFrameOptionsChanged.quality ? 'danger' : ''"
                                @click="batchChangeFrameOption('quality')"/>
                        </el-form-item>
                        <el-form-item label="Method">
                            <el-input-number 
                                v-model="batchFrameOptions.method"
                                :min="0" :max="6" :step="1"
                                @change="batchFrameOptionsChanged.method = true"/>
                            <el-button
                                icon="el-icon-check" plain
                                :type="batchFrameOptionsChanged.method ? 'danger' : ''"
                                @click="batchChangeFrameOption('method')"/>
                        </el-form-item>
                        <el-form-item label="Lossless">
                            <el-switch
                                v-model="batchFrameOptions.lossless"
                                @change="batchFrameOptionsChanged.lossless = true"/>
                            <el-button
                                icon="el-icon-check" plain
                                :type="batchFrameOptionsChanged.lossless ? 'danger' : ''"
                                @click="batchChangeFrameOption('lossless')"/>
                        </el-form-item>
                        <el-form-item label="Exact">
                            <el-switch
                                v-model="batchFrameOptions.exact"
                                @change="batchFrameOptionsChanged.exact = true"/>
                            <el-button
                                icon="el-icon-check" plain
                                :type="batchFrameOptionsChanged.exact ? 'danger' : ''"
                                @click="batchChangeFrameOption('exact')"/>
                        </el-form-item>
                    </el-form>
                </div>
                <div class="webp-container" v-loading="loading">
                    <img v-if="webp.src !== ''" :src="webp.src" @click="downloadWebp"/>
                </div>
            </div>
            <div class="frames-container">
                <el-empty v-if="!frames.length" description="Drag and drop images to here"/>
                <div v-else class="frames-wrap">
                    <image-frame
                        v-for="frame of frames"
                        :key="frame.src"
                        :src="frame.src"
                        :name="frame.name"
                        v-model="frame.options"/>
                    <i v-for="i in 10"></i>
                </div>
            </div>
            <a href="https://github.com/xiaozhuai/webp_encoder" class="github-corner" target="_blank" aria-label="View source on GitHub">
                <svg width="80" height="80" viewBox="0 0 250 250" style="fill:#151513; color:#fff; position: absolute; top: 0; border: 0; right: 0;" aria-hidden="true">
                  <path d="M0,0 L115,115 L130,115 L142,142 L250,250 L250,0 Z"></path>
                  <path d="M128.3,109.0 C113.8,99.7 119.0,89.6 119.0,89.6 C122.0,82.7 120.5,78.6 120.5,78.6 C119.2,72.0 123.4,76.3 123.4,76.3 C127.3,80.9 125.5,87.3 125.5,87.3 C122.9,97.6 130.6,101.9 134.4,103.2" fill="currentColor" style="transform-origin: 130px 106px;" class="octo-arm"></path>
                  <path d="M115.0,115.0 C114.9,115.1 118.7,116.5 119.8,115.4 L133.7,101.6 C136.9,99.2 139.9,98.4 142.2,98.6 C133.8,88.0 127.5,74.4 143.8,58.0 C148.5,53.4 154.0,51.2 159.7,51.0 C160.3,49.4 163.2,43.6 171.4,40.1 C171.4,40.1 176.1,42.5 178.8,56.2 C183.1,58.6 187.2,61.8 190.9,65.4 C194.5,69.0 197.7,73.2 200.1,77.6 C213.8,80.2 216.3,84.9 216.3,84.9 C212.7,93.1 206.9,96.0 205.4,96.6 C205.1,102.4 203.0,107.8 198.3,112.5 C181.9,128.9 168.3,122.5 157.7,114.1 C157.9,116.9 156.7,120.9 152.7,124.9 L141.0,136.5 C139.8,137.7 141.6,141.9 141.8,141.8 Z" fill="currentColor" class="octo-body"></path>
                </svg>
            </a>
            <div
                class="drag-layer"
                v-if="dragging"
                @drop.prevent.self.stop="onDropFiles"
                @dragover.prevent.self.stop=""
                @dragleave.prevent.self.stop="dragging = false;">
                Drag & drop images to here!
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
            batchFrameOptions: {
                duration: 100,
                quality: 100,
                method: 0,
                lossless: false,
                exact: false,
            },
            batchFrameOptionsChanged: {
                duration: false,
                quality: false,
                method: false,
                lossless: false,
                exact: false,
            },
            frames: [],
            webp: {
                src: '',
                size: 0,
            },
            loading: true,
            progress: 0,
            dragging: false,
        };
    },
    computed: {
        readableWebpSize() {
            return this.toHumanReadableSize(this.webp.size);
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
            message: 'Drag and drop image to here to generate a webp!',
            position: 'bottom-right',
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
                    options: {...this.batchFrameOptions},
                });
            }
            this.frames = this.sortFrames(frames);
        },
        batchChangeFrameOption(key) {
            let value = this.batchFrameOptions[key];
            let frames = [...this.frames];
            for (let frame of frames) {
                frame.options[key] = value;
            }
            this.batchFrameOptionsChanged[key] = false;
        },
        toHumanReadableSize(bytes, precision = 2) {
            if (typeof bytes !== 'number' || isNaN(bytes) || !isFinite(bytes)) return '-';
            const units = ['bytes', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB', 'BB'];
            let num = bytes;
            let unit = 0;
            if (bytes !== 0) {
                unit = Math.floor(Math.log(bytes) / Math.log(1024));
                num = bytes / Math.pow(1024, Math.floor(unit));
            }
            if (unit === 0) {
                return num + ' ' + units[unit];
            } else {
                return num.toFixed(precision) + ' ' + units[unit];
            }
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
                this.$message.error('No frames, drag and drop image frames to continue');
                return;
            }
            this.loading = true;
            this.progress = 0;
            this.webp = {
                src: '',
                size: 0,
            };
            await this.$nextTick();

            try {
                let {url, size} = await WebpEncoder.encode(this.frames, progress => {
                    this.progress = progress;
                }, this.fileOptions, WebpEncoder.ENCODE_RET_URL);
                this.webp = {src: url, size};
                this.progress = 100;
                this.loading = false;
            } catch (e) {
                console.error(e);
                this.$message.error('Encode webp failed! Please check logs in console!');
                this.progress = 0;
                this.loading = false;
            }
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
